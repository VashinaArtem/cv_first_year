# Программа для сегментации объектов на микроскопических изображениях бактериальных культур

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=c%2B%2B)
![OpenCV](https://img.shields.io/badge/OpenCV-4.x-green?logo=opencv)
![CMake](https://img.shields.io/badge/CMake-3.15%2B-red?logo=cmake)
![vcpkg](https://img.shields.io/badge/vcpkg-manifest-blueviolet)
![Platform](https://img.shields.io/badge/platform-Windows%20x64-lightgrey?logo=windows)

Детерминированный алгоритм пакетной сегментации бактерий на микроскопических снимках. Программа автоматически отделяет области с бактериями от фона, артефактов и посторонних объектов, выдавая бинарные маски.

**Метрики на тестовом датасете:** IoU = 0.888 ± 0.032 · R = 0.921 ± 0.028

---

## Пример работы

| Входное изображение | Результат сегментации |
|:---:|:---:|
| ![input](<!-- вставь путь к картинке, например: docs/images/input_example.jpg -->) | ![output](<!-- вставь путь к картинке, например: docs/images/output_example.png -->) |

> Белым — совпадение с эталоном, красным — ложные срабатывания, зелёным — пропущенные объекты.

---

## Алгоритм обработки

```
Входное изображение (JPG/PNG)
        │
        ▼
  1. Grayscale + GaussianBlur (5×5)
        │
        ▼
  2. Локальная бинаризация Оцу по тайлам (300×300 px)
        │
        ▼
  3. Морфологическое открытие (эллипс 3×3)
        │
        ▼
  4. Геометрическая фильтрация контуров (площадь, периметр, форма)
        │
        ▼
  5. Фильтрация по яркости (порог μ + 1.2σ)
        │
        ▼
  6. Разделение слипшихся объектов — Watershed
        │
        ▼
  Бинарная маска PNG → algo_pred/
```

---

## Требования

| Инструмент | Версия |
|---|---|
| Visual Studio 2022 (MSVC, workload *Desktop C++*) | 17.x |
| CMake | 3.15+ |
| vcpkg | любая актуальная |
| Git | любая |
| Doxygen | опционально, для документации |

---

## Установка окружения (Windows)

<details>
<summary>Развернуть подробные инструкции</summary>

**1. Visual Studio 2022**
```powershell
winget install Microsoft.VisualStudio.2022.Community
```
Проверь, что установлен workload **Desktop development with C++**.

**2. CMake**
```powershell
winget install Kitware.CMake
```

**3. Git**
```powershell
winget install Git.Git
```

**4. vcpkg** (клонируй в любое место, здесь пример — `C:/programs/vcpkg`)
```powershell
git clone https://github.com/microsoft/vcpkg C:/programs/vcpkg
$env:VCPKG_ROOT = "C:/programs/vcpkg"   # замени на свой путь
& "$env:VCPKG_ROOT/bootstrap-vcpkg.bat"
& "$env:VCPKG_ROOT/vcpkg" integrate install
```

**5. OpenCV через vcpkg**
```powershell
& "$env:VCPKG_ROOT/vcpkg" install opencv[core,imgproc,highgui,imgcodecs]:x64-windows
```

**6. Doxygen (опционально)**
```powershell
winget install Doxygen.Doxygen
```

</details>

---

## Быстрый старт

```powershell
# 1. Клонируй репозиторий
git clone https://github.com/VashinaArtem/cv_first_year.git
cd cv_first_year

# 2. Открой VS Dev Shell x64
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64

# 3. Укажи путь к vcpkg (замени на свой)
$env:VCPKG_ROOT = "C:/programs/vcpkg"

# 4. Сконфигурируй
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DCMAKE_BUILD_TYPE=Release

# 5. Собери
cmake --build build-msvc --config Release -j

# 6. Установи
cmake --install build-msvc --config Release --prefix .\install
```

---

## Запуск

После установки в папке `install/` будет готовый к запуску пакет:

```
install/
├── bin/
│   └── cells_pipeline.exe
└── control/
    └── *.jpg   ← входные изображения
```

```powershell
cd install
.\bin\cells_pipeline.exe
```

Программа читает `.jpg`/`.jpeg`/`.png` из папки `control`, автоматически создаёт папку `algo_pred` и сохраняет туда бинарные PNG-маски с теми же именами файлов.

**Ожидаемый вывод:**
```
Successfully processed: control\1.1.1.jpg
Successfully processed: control\1.1.2.jpg
...
```

| Код возврата | Значение |
|---|---|
| `0` | Успешная обработка всех изображений |
| `-1` | Папка `control` не найдена |

---

## Структура проекта

```
cv_first_year/
├── cells_pipeline.cpp   # основной алгоритм
├── CMakeLists.txt        # сборка (CMake 3.15+)
├── Doxyfile              # конфигурация документации
├── vcpkg.json            # зависимости (OpenCV через vcpkg)
├── control/              # входные изображения
└── docs/                 # сгенерированная документация (не в git)
```

---

## Документация (Doxygen)

```powershell
cmake --build build-msvc --target docs --config Release
start .\docs\latex\refman.pdf
```

> Цель `docs` доступна только если Doxygen установлен и найден в PATH.

---

## Типичные проблемы

<details>
<summary>CMake не находит OpenCV</summary>

- Проверь переменную: `$env:VCPKG_ROOT` должна указывать на реальную папку vcpkg
- Убедись, что порт установлен: `vcpkg install opencv[core,imgproc,highgui,imgcodecs]:x64-windows`
- Используй совпадающую разрядность: флаг `-A x64` и триплет `x64-windows`
- Запускай cmake из **VS Dev Shell x64**

</details>

<details>
<summary>Команда `cl` не найдена</summary>

- Запусти PowerShell через **VS Dev Shell x64** (см. команду выше)
- Проверь, что установлен workload *Desktop development with C++* в Visual Studio

</details>

<details>
<summary>Цель `docs` отсутствует при сборке</summary>

- Установи Doxygen и добавь в PATH
- Пересобери конфигурацию CMake после установки (`cmake -S . -B build-msvc ...`)

</details>

<details>
<summary>Чистая пересборка</summary>

```powershell
Remove-Item -Recurse -Force build-msvc
# затем повтори шаги конфигурации и сборки
```

</details>

---

## Технические детали

| Параметр | Значение |
|---|---|
| Язык | C++17 |
| Компилятор | MSVC (Visual Studio 17 2022) |
| Платформа | Windows x64 |
| Зависимости | `opencv_core`, `opencv_imgproc`, `opencv_imgcodecs`, `opencv_highgui` |
| Менеджер пакетов | vcpkg (manifest mode, `vcpkg.json`) |
| Документация | Doxygen → LaTeX → PDF |
