## cells_pipeline

Гайд по сборке, запуску и документации проекта на Windows (MSVC + CMake + vcpkg).

— C++17, OpenCV (через vcpkg), CMake, MSVC 2022  
— Документация генерируется Doxygen (опционально)


### TL;DR (быстрый старт)

Открой PowerShell и запусти среду разработчика VS x64:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64
```

Проверь компилятор:

```powershell
cl
```

Сконфигурируй CMake (Visual Studio 2022 + vcpkg toolchain, x64):

```powershell
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/programs/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DCMAKE_BUILD_TYPE=Release
```

Собери:

```powershell
cmake --build build-msvc --config Release -j
```

Запусти приложение:

```powershell
.\build-msvc\Release\cells_pipeline.exe
```

Сгенерируй документацию (если установлен Doxygen):

```powershell
cmake --build build-msvc --target docs --config Release
start .\docs\html\index.html
```


### Что внутри

- Исполняемый файл: `cells_pipeline` (таргет из `CMakeLists.txt`)
- Зависимости: `opencv_core`, `opencv_imgproc`, `opencv_imgcodecs`, `opencv_highgui` (через vcpkg)
- Документация: цель CMake `docs` вызывает `Doxygen` по `Doxyfile`


### Требования

- Visual Studio 2022 (или Build Tools) с инструментами C++ (x64, MSVC)
- CMake 3.15+
- Git
- vcpkg
- OpenCV (ставится через vcpkg)
- Doxygen (опционально, для генерации документации)


### Установка окружения (Windows, PowerShell)

1) Visual Studio 2022 Community (или Build Tools)
- Скачай с сайта Microsoft или:
  - Через winget (потребуется доустановка компонентов в UI):
    ```powershell
    winget install Microsoft.VisualStudio.2022.Community
    ```
  - Проверь, что установлен workload "Desktop development with C++"

2) CMake
```powershell
winget install Kitware.CMake
```

3) Git
```powershell
winget install Git.Git
```

4) vcpkg
```powershell
git clone https://github.com/microsoft/vcpkg C:/programs/vcpkg
C:/programs/vcpkg/bootstrap-vcpkg.bat
C:/programs/vcpkg/vcpkg integrate install
```

5) OpenCV через vcpkg (x64-windows)
```powershell
C:/programs/vcpkg/vcpkg install opencv[core,imgproc,highgui,imgcodecs]:x64-windows
```

6) (Опционально) Doxygen
- Через winget:
  ```powershell
  winget install Doxygen.Doxygen
  ```
- Или через Chocolatey (если установлен):
  ```powershell
  choco install doxygen.install -y
  ```


### Сборка проекта (подробно)

1) Открой VS Dev Shell x64:
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64
```

2) Конфигурация CMake:
```powershell
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/programs/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DCMAKE_BUILD_TYPE=Release
```

3) Сборка:
```powershell
cmake --build build-msvc --config Release -j
```

4) (Опционально) Открыть решение в Visual Studio:
```powershell
start .\build-msvc\Polevoy_sem1.sln
```


### Запуск

```powershell
.\build-msvc\Release\cells_pipeline.exe
```

Если программе нужны аргументы/ввод, смотри комментарии в `cells_pipeline.cpp` или обработку CLI (если есть).


### Документация (Doxygen)

Генерация html-доков из корня проекта:
```powershell
cmake --build build-msvc --target docs --config Release
```

Открыть:
```powershell
start .\docs\html\index.html
```

Примечания:
- Цель `docs` доступна только если `doxygen` в PATH (`CMakeLists.txt` проверяет `find_package(Doxygen)`).
- Настройки берутся из `Doxyfile` (HTML включён, Graphviz не обязателен).


### Типичные проблемы и решения

- CMake не находит OpenCV
  - Проверь путь к toolchain: `-DCMAKE_TOOLCHAIN_FILE=C:/programs/vcpkg/scripts/buildsystems/vcpkg.cmake`
  - Убедись, что установлен порт: `vcpkg install opencv[core,imgproc,highgui,imgcodecs]:x64-windows`
  - Соответствие разрядности: используй `-A x64` и триплет `x64-windows`
  - Запускай команды из VS Dev Shell x64

- Команда `cl` не находится
  - Запусти DevShell: см. команду запуска VS Dev Shell
  - Установи/доустанови workload "Desktop development with C++" в Visual Studio

- Цель `docs` отсутствует
  - Установи Doxygen и добавь в PATH (см. раздел про установку)
  - Пересобери конфигурацию CMake после установки Doxygen

- Сборка Debug/Release
  - Для Debug используй: `cmake --build build-msvc --config Debug`

- Чистая пересборка
  - Удали папку `build-msvc` и запусти конфигурацию/сборку заново


### Технические детали

- Стандарт C++: C++17 (`set(CMAKE_CXX_STANDARD 17)`)
- Генератор: Visual Studio 17 2022, платформа x64
- Основные библиотеки: `opencv_core`, `opencv_imgproc`, `opencv_imgcodecs`, `opencv_highgui`
- Документация: CMake-цель `docs` вызывает Doxygen по `Doxyfile`


Удачной сборки и приятной работы с cells_pipeline! 🚀


