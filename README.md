# Bacterial Cell Segmentation on Microscopic Images

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=c%2B%2B)
![OpenCV](https://img.shields.io/badge/OpenCV-4.x-green?logo=opencv)
![CMake](https://img.shields.io/badge/CMake-3.15%2B-red?logo=cmake)
![vcpkg](https://img.shields.io/badge/vcpkg-manifest-blueviolet)
![Platform](https://img.shields.io/badge/platform-Windows%20x64-lightgrey?logo=windows)

A deterministic batch segmentation pipeline for bacteria on brightfield microscopy images. The program automatically separates bacterial regions from background, artifacts, and foreign objects, producing binary masks.

**Metrics on test dataset:** IoU = 0.888 ± 0.032 · R = 0.921 ± 0.028

---

## Example

| Input image | Segmentation result |
|:---:|:---:|
| ![input](<!-- insert path, e.g.: docs/images/input_example.jpg -->) | ![output](<!-- insert path, e.g.: docs/images/output_example.png -->) |

> White — matches ground truth, red — false positives, green — missed objects.

---

## Pipeline

```
Input image (JPG/PNG)
        │
        ▼
  1. Grayscale + GaussianBlur (5×5)
        │
        ▼
  2. Local Otsu thresholding per tile (300×300 px)
        │
        ▼
  3. Morphological opening (ellipse 3×3)
        │
        ▼
  4. Geometric contour filtering (area, perimeter, shape ratio)
        │
        ▼
  5. Brightness filtering (threshold μ + 1.2σ)
        │
        ▼
  6. Splitting merged objects — Watershed
        │
        ▼
  Binary PNG mask → algo_pred/
```

---

## Requirements

| Tool | Version |
|---|---|
| Visual Studio 2022 (MSVC, workload *Desktop C++*) | 17.x |
| CMake | 3.15+ |
| vcpkg | any recent |
| Git | any |
| Doxygen | optional, for documentation |

---

## Environment Setup (Windows)

<details>
<summary>Expand full setup instructions</summary>

**1. Visual Studio 2022**
```powershell
winget install Microsoft.VisualStudio.2022.Community
```
Make sure the **Desktop development with C++** workload is installed.

**2. CMake**
```powershell
winget install Kitware.CMake
```

**3. Git**
```powershell
winget install Git.Git
```

**4. vcpkg** (clone anywhere, example uses `C:/programs/vcpkg`)
```powershell
git clone https://github.com/microsoft/vcpkg C:/programs/vcpkg
$env:VCPKG_ROOT = "C:/programs/vcpkg"   # replace with your path
& "$env:VCPKG_ROOT/bootstrap-vcpkg.bat"
& "$env:VCPKG_ROOT/vcpkg" integrate install
```

**5. OpenCV via vcpkg**
```powershell
& "$env:VCPKG_ROOT/vcpkg" install opencv[core,imgproc,highgui,imgcodecs]:x64-windows
```

**6. Doxygen (optional)**
```powershell
winget install Doxygen.Doxygen
```

</details>

---

## Quick Start

```powershell
# 1. Clone the repository
git clone https://github.com/VashinaArtem/cv_first_year.git
cd cv_first_year

# 2. Open VS Dev Shell x64
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64

# 3. Set your vcpkg path
$env:VCPKG_ROOT = "C:/programs/vcpkg"

# 4. Configure
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DCMAKE_BUILD_TYPE=Release

# 5. Build
cmake --build build-msvc --config Release -j

# 6. Install
cmake --install build-msvc --config Release --prefix .\install
```

---

## Running

After installation, `install/` contains a ready-to-run package:

```
install/
├── bin/
│   └── cells_pipeline.exe
└── control/
    └── *.jpg   ← input images
```

```powershell
cd install
.\bin\cells_pipeline.exe
```

The program reads `.jpg`/`.jpeg`/`.png` files from the `control` folder, automatically creates `algo_pred`, and saves binary PNG masks there with the same filenames.

**Expected output:**
```
Successfully processed: control\1.1.1.jpg
Successfully processed: control\1.1.2.jpg
...
```

| Exit code | Meaning |
|---|---|
| `0` | All images processed successfully |
| `-1` | `control` folder not found |

---

## Project Structure

```
cv_first_year/
├── cells_pipeline.cpp   # segmentation pipeline
├── CMakeLists.txt        # build config (CMake 3.15+)
├── Doxyfile              # documentation config
├── vcpkg.json            # dependencies (OpenCV via vcpkg)
├── control/              # input images
└── docs/                 # generated documentation (not tracked in git)
```

---

## Documentation (Doxygen)

```powershell
cmake --build build-msvc --target docs --config Release
start .\docs\latex\refman.pdf
```

> The `docs` target is only available if Doxygen is installed and found in PATH.

---

## Troubleshooting

<details>
<summary>CMake can't find OpenCV</summary>

- Check that `$env:VCPKG_ROOT` points to your actual vcpkg folder
- Make sure the port is installed: `vcpkg install opencv[core,imgproc,highgui,imgcodecs]:x64-windows`
- Use matching architecture: `-A x64` flag and `x64-windows` triplet
- Run cmake from **VS Dev Shell x64**

</details>

<details>
<summary>Command `cl` not found</summary>

- Open PowerShell via **VS Dev Shell x64** (see command above)
- Verify that *Desktop development with C++* workload is installed in Visual Studio

</details>

<details>
<summary>`docs` target missing during build</summary>

- Install Doxygen and add it to PATH
- Re-run CMake configuration after installing Doxygen (`cmake -S . -B build-msvc ...`)

</details>

<details>
<summary>Clean rebuild</summary>

```powershell
Remove-Item -Recurse -Force build-msvc
# then repeat the configure and build steps
```

</details>

---

## Technical Details

| Parameter | Value |
|---|---|
| Language | C++17 |
| Compiler | MSVC (Visual Studio 17 2022) |
| Platform | Windows x64 |
| Dependencies | `opencv_core`, `opencv_imgproc`, `opencv_imgcodecs`, `opencv_highgui` |
| Package manager | vcpkg (manifest mode, `vcpkg.json`) |
| Documentation | Doxygen → LaTeX → PDF |
