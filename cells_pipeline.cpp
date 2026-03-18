/**
 * @file cells_pipeline.cpp
 * @brief Пайплайн сегментации клеток на микроснимках: локальный Оцу, морфология, фильтрация и watershed.
 *
 * @details
 * Программа пакетно обрабатывает все изображения из каталога `control` и сохраняет
 * бинарные маски результатов сегментации в каталог `algo_pred` (с теми же именами файлов, расширение PNG).
 * Основные этапы:
 *  - преобразование в оттенки серого и сглаживание (GaussianBlur);
 *  - локальная бинаризация по Оцу в тайлах;
 *  - морфологическое открытие для подавления шума;
 *  - поиск контуров и геометрическая фильтрация по площади/периметру/отношению;
 *  - фильтрация по средней яркости внутри контура (относительно μ±σ);
 *  - разделение слипающихся объектов алгоритмом watershed по карте расстояний;
 *  - сохранение итоговой маски.
 *
 * @note Требования: C++17 (std::filesystem) и OpenCV 4+.
 */
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <numeric>
#include <cmath>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

/**
 * @brief Локальная бинаризация изображения методом Оцу по тайлам.
 *
 * @param src Входное изображение в градациях серого (CV_8UC1).
 * @param tileSize Размер квадрата тайла (в пикселях), которым покрывается изображение.
 * @return Бинарная маска (CV_8UC1, значения 0/255), собранная из порогов по каждому тайлу.
 *
 * @details
 * Изображение разбивается на неперекрывающиеся ROI размером `tileSize×tileSize` (последние тайлы
 * могут быть меньше у правого/нижнего краёв). Для каждого ROI применяется `cv::threshold` с
 * флагами `THRESH_BINARY_INV | THRESH_OTSU`, после чего результат копируется в итоговую маску.
 *
 * @warning Ожидается, что `src` имеет тип CV_8UC1. При других типах результаты не определены.
 */
cv::Mat localOtsuThreshold(const cv::Mat& src, int tileSize) {
    cv::Mat result = cv::Mat::zeros(src.size(), CV_8UC1);
    for (int y = 0; y < src.rows; y += tileSize) {
        for (int x = 0; x < src.cols; x += tileSize) {
            int width = std::min(tileSize, src.cols - x);
            int height = std::min(tileSize, src.rows - y);
            cv::Rect roi(x, y, width, height);
            cv::Mat tile = src(roi);
            cv::Mat tileThresh;
            cv::threshold(tile, tileThresh, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
            tileThresh.copyTo(result(roi));
        }
    }
    return result;
}

/**
 * @brief Вычисляет среднюю яркость в пределах каждого контура и фильтрует их.
 *
 * @param image Одноканальное изображение (CV_8UC1), по которому считается яркость.
 * @param contours Входной набор контуров для оценки.
 * @param[out] brightnessValues Массив средних яркостей (по одному на контур).
 * @param[out] meanBrightness Среднее значение яркости по всем контурам.
 * @param[out] stdDevBrightness Среднеквадратичное отклонение яркости.
 * @return Контуры, прошедшие фильтрацию по условию brightness <= (μ + σ) * 1.2.
 *
 * @details
 * Для каждого контура строится маска, по которой считается `cv::mean(image, mask)`.
 * Затем вычисляются агрегаты μ и σ, и отбираются контуры, не являющиеся слишком яркими.
 */
std::vector<std::vector<cv::Point>> calculateMeanColorForContours(
    const cv::Mat& image,
    const std::vector<std::vector<cv::Point>>& contours,
    std::vector<float>& brightnessValues,
    float& meanBrightness,
    float& stdDevBrightness)
{
    std::vector<std::vector<cv::Point>> result;
    brightnessValues.clear();

    for (const auto& contour : contours) {
        cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
        cv::drawContours(mask, std::vector<std::vector<cv::Point>>{contour}, -1, 255, cv::FILLED);
        cv::Scalar meanVal = cv::mean(image, mask);
        brightnessValues.push_back(static_cast<float>(meanVal[0]));
    }

    float sum = std::accumulate(brightnessValues.begin(), brightnessValues.end(), 0.0f);
    meanBrightness = sum / brightnessValues.size();

    float sq_sum = std::inner_product(brightnessValues.begin(), brightnessValues.end(), brightnessValues.begin(), 0.0f);
    stdDevBrightness = std::sqrt(sq_sum / brightnessValues.size() - meanBrightness * meanBrightness);

    float upper = meanBrightness + stdDevBrightness;

    for (size_t i = 0; i < contours.size(); ++i) {
        if (brightnessValues[i] <= upper * 1.2) {
            result.push_back(contours[i]);
        }
    }

    return result;
}

/**
 * @brief Разделяет слипшиеся объекты на бинарной маске с помощью алгоритма watershed.
 *
 * @param input_mask Входная бинарная маска (CV_8UC1, значения 0/255).
 * @param[out] output_mask Результирующая бинарная маска после разделения объектов.
 *
 * @details
 * Маска очищается морфологическим открытием, по ней считается карта расстояний (L2),
 * затем локальные максимумы выделяются порогом и используются как маркеры для `cv::watershed`.
 * Границы, помеченные значением -1 в маркерах, зануляются, что визуально разделяет объекты.
 */
void processWatershed(const cv::Mat& input_mask, cv::Mat& output_mask) {
    // Бинаризация
    cv::Mat binary;
    cv::threshold(input_mask, binary, 127, 255, cv::THRESH_BINARY);

    // Удаление шумов
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);

    // Расстояние до ближайшего нулевого пикселя
    cv::Mat distTransform;
    cv::distanceTransform(binary, distTransform, cv::DIST_L2, 5);

    // Нормализация и пороговая обработка
    cv::normalize(distTransform, distTransform, 0, 1.0, cv::NORM_MINMAX);
    cv::Mat distBinary;
    cv::threshold(distTransform, distBinary, 0.4, 1.0, cv::THRESH_BINARY);
    distBinary.convertTo(distBinary, CV_8U);

    // Поиск контуров
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(distBinary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Создание меток
    cv::Mat markers = cv::Mat::zeros(distBinary.size(), CV_32S);
    for (size_t i = 0; i < contours.size(); i++) {
        cv::drawContours(markers, contours, static_cast<int>(i), cv::Scalar(static_cast<int>(i) + 1), -1);
    }

    // Применение алгоритма Watershed
    cv::Mat maskColor;
    cv::cvtColor(binary, maskColor, cv::COLOR_GRAY2BGR);
    cv::watershed(maskColor, markers);

    // Рисование границ
    cv::Mat result = binary.clone();
    for (int y = 0; y < markers.rows; y++) {
        for (int x = 0; x < markers.cols; x++) {
            if (markers.at<int>(y, x) == -1) {
                result.at<uchar>(y, x) = 0;
            }
        }
    }

    // Утолщение границ
    cv::Mat boundaries = (result == 0);
    cv::dilate(boundaries, boundaries, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));
    result.setTo(0, boundaries);

    output_mask = result;
}

/**
 * @brief Точка входа. Пакетно сегментирует изображения из каталога `control` и сохраняет маски в `algo_pred`.
 *
 * @return Код возврата процесса: 0 — успешно; -1 — входной каталог отсутствует.
 *
 * @par Вход/выход
 *  - Вход: файлы с расширениями .jpg/.jpeg/.png из `control`.
 *  - Выход: бинарные маски PNG в `algo_pred` с теми же базовыми именами.
 *
 * @note Пороговые и геометрические параметры подобраны эмпирически и могут требовать настройки под датасет.
 */
int main() {
    const std::string inputFolder = "control";
    const std::string outputFolder = "algo_pred";

    if (!fs::exists(inputFolder)) {
        std::cerr << "Error: Input folder '" << inputFolder << "' does not exist!" << std::endl;
        return -1;
    }
    fs::create_directories(outputFolder);

    for (const auto& entry : fs::directory_iterator(inputFolder)) {
        const std::string filename = entry.path().string();
        const std::string ext = entry.path().extension().string();

        if (ext != ".jpg" && ext != ".jpeg" && ext != ".png") {
            std::cout << "Skipping non-image file: " << filename << std::endl;
            continue;
        }

        cv::Mat img = cv::imread(filename);
        if (img.empty()) {
            std::cout << "Could not load image: " << filename << std::endl;
            continue;
        }

        // 1. Предварительная обработка
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

        // 2. Локальное пороговое преобразование Оцу
        cv::Mat localOtsu = localOtsuThreshold(gray, 300);

        // 3. Морфологическое открытие
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::Mat morph;
        cv::morphologyEx(localOtsu, morph, cv::MORPH_OPEN, kernel);

        // 4. Поиск контуров и геометрическая фильтрация
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(morph, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        const double minArea = 90.0, maxArea = 3000.0;
        const double minPerimeter = 30.0, maxPerimeter = 3000.0;
        const double minRatio = 0.1, maxRatio = 0.6;

        std::vector<std::vector<cv::Point>> filtered;
        for (const auto& contour : contours) {
            const double area = cv::contourArea(contour);
            const double perimeter = cv::arcLength(contour, true);

            if (area > minArea && area < maxArea &&
                perimeter > minPerimeter && perimeter < maxPerimeter &&
                area != 0 && (perimeter / area > minRatio && perimeter / area < maxRatio))
            {
                filtered.push_back(contour);
            }
        }

        // 5. Фильтрация по яркости
        std::vector<float> brightnessValues;
        float mean, stddev;
        auto brightnessFiltered = calculateMeanColorForContours(gray, filtered, brightnessValues, mean, stddev);

        // Создание маски
        cv::Mat brightnessMask = cv::Mat::zeros(img.size(), CV_8UC1);
        for (const auto& contour : brightnessFiltered) {
            cv::drawContours(brightnessMask, std::vector<std::vector<cv::Point>>{contour}, -1, 255, cv::FILLED);
        }

        // 6. Применение алгоритма Watershed
        cv::Mat watershedResult;
        processWatershed(brightnessMask, watershedResult);

        // Сохранение результата
        const std::string outputPath = outputFolder + "/" + entry.path().stem().string() + ".png";
        cv::imwrite(outputPath, watershedResult);
        std::cout << "Successfully processed: " << filename << std::endl;
    }

    return 0;
}