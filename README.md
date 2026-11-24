apitk: ITK-SNAP with Label Paint mode
=====================================

**apitk** is a personal fork of [ITK-SNAP](http://www.itksnap.org) 4.4.0. It
adds one feature: **Label Paint mode**, a brush that relabels whole 3D
connected components in a segmentation with one click. Everything else is the
same as upstream ITK-SNAP 4.4.0.

- Repository: <https://github.com/Arshya-Guru/apitk> (branch `apitk`)
- Upstream: <https://github.com/pyushkevich/itksnap>
- License: GNU GPL v3 (see [`COPYING`](COPYING)), same as upstream

Contents:
[Quick start](#quick-start) ·
[Using Label Paint](#using-label-paint) ·
[Dependencies](#dependencies) ·
[Building on Linux](#building-on-linux) ·
[Building on macOS (Apple Silicon)](#building-on-macos-apple-silicon) ·
[Windows](#windows) ·
[Upstream ITK-SNAP README](#upstream-itk-snap-readme)

---

## Quick start

```bash
git clone --recursive -b apitk https://github.com/Arshya-Guru/apitk.git
cd apitk
# Already cloned without --recursive? Run:
git submodule update --init --recursive     # c3d, greedy, digestible

# Build ITK v5.4.0 and VTK 9.3.1 first (see below), then:
cmake -S . -B build -G Ninja \
  -DITK_DIR=<itk>/build \
  -DVTK_DIR=<vtk>/install/lib/cmake/vtk-9.3 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build -t ITK-SNAP
```

The program is the executable `build/ITK-SNAP` on both Linux and macOS.
To launch it with an `apitk` command, see the platform sections below.

**Don't want to build?** Prebuilt Windows, Linux and macOS packages are
attached to each run of the **apitk build** workflow in the repo's Actions
tab. See [Windows](#windows) for how to download them.

---

## Using Label Paint

Label Paint takes the place of **Polygon** mode in the GUI. It sits in the
same toolbar slot, in the same Tools menu entry ("Label Paint Mode"), and uses
the same **`3`** keyboard shortcut.

1. Load an image and a segmentation.
2. Press **`3`** (or click the Label Paint toolbar button).
3. In the main control panel, choose the **active label** (the label you want
   to assign) and set **Paint over**. Paint over decides which existing labels
   can be changed.
4. **Left-click** on the segmentation. Every label under the brush that passes
   the Paint over filter has its **whole 3D connected component** relabelled
   to the active label, including the parts on other slices.
5. Made a mistake? Use **Undo** (Ctrl/Cmd+Z). Redo also works.

Details:

- **Brush settings** (Label Paint tool inspector): shape (square/round), size
  (`+`/`-` keys), *3D brush*, *Isotropic*, and *Cursor chases brush*. These
  work the same way as in the paintbrush. The brush only chooses *which*
  components get picked. The components themselves are always found in 3D.
- **Connectivity:** by default, voxels count as connected only if they share
  a face (6-connectivity). Tick **26-connectivity** to also connect voxels
  that touch at an edge or corner.
- **Background:** label 0 (clear) is never treated as a component, unless
  Paint over is set to the clear label on purpose.
- **Right-click does nothing** in this mode.
- The Polygon mode code is still in the source tree, but there is no way to
  reach it from the GUI.

The feature was added in commit `54780425`. Most of the code is in
`GUI/Model/LabelPaintModel.*`, `GUI/Model/LabelPaintSettingsModel.*`,
`GUI/Qt/Components/LabelPaintToolPanel.*` and
`GUI/Qt/View/LabelPaintInteractionMode.*`.

---

## Dependencies

These versions match the upstream CI ([`.github/workflows/build.yml`](.github/workflows/build.yml)):

| Dependency | Version / notes |
|---|---|
| CMake | ≥ 3.16 |
| Ninja | any recent version |
| ITK | **v5.4.0**, configured with `-DModule_MorphologicalContourInterpolation=ON` |
| VTK | **9.3.1**: static (`BUILD_SHARED_LIBS=OFF`), Qt group enabled, `GUISupportQtQuick` and `GUISupportQtSQL` turned off |
| Qt | **Qt 6**. CI uses 6.8.1. Homebrew Qt 6.9.3 works on macOS. |
| libcurl, libssh | system packages |
| Git submodules | `Submodules/c3d`, `Submodules/greedy`, `Submodules/digestible` |

ITK-SNAP will not compile without the submodules:

```bash
git submodule update --init --recursive
```

> **ITK_DIR must point at the ITK *build* directory, not the install
> directory.** ITK's `install` step leaves out a file (`vnl_vector_ref.hxx`),
> so upstream CI also uses the build tree.

The ITK and VTK builds together take about an hour on an 8-core M-series Mac and
need about 10–15 GB of disk space.

---

## Building on Linux

This follows the CI recipe in `.github/workflows/build.yml` (Ubuntu 22.04).

```bash
# System packages
sudo apt install build-essential git cmake ninja-build \
                 libcurl4-openssl-dev libssh-dev
# Qt 6: use your distro's Qt 6 dev packages or the Qt online installer
# (CI uses Qt 6.8.1, desktop, gcc_64).

DEPS=$HOME/apitk-deps      # anywhere you like
mkdir -p $DEPS && cd $DEPS

# VTK 9.3.1
git clone --recursive -b v9.3.1 https://github.com/Kitware/VTK.git vtk
cmake -S vtk -B vtk/build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$DEPS/vtk/install \
  -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=OFF \
  -DVTK_GROUP_ENABLE_Qt=YES -DVTK_QT_VERSION=6 \
  -DVTK_MODULE_ENABLE_VTK_GUISupportQtQuick=NO \
  -DVTK_MODULE_ENABLE_VTK_GUISupportQtSQL=NO \
  -DVTK_MODULE_ENABLE_VTK_RenderingExternal=WANT \
  -DVTK_REQUIRED_OBJCXX_FLAGS=
  # add -DCMAKE_PREFIX_PATH=<qt6 prefix> if Qt is not on the default path
cmake --build vtk/build -t install

# ITK v5.4.0
git clone --recursive -b v5.4.0 https://github.com/InsightSoftwareConsortium/ITK.git itk
cmake -S itk -B itk/build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF \
  -DModule_MorphologicalContourInterpolation=ON
cmake --build itk/build

# apitk
cd /path/to/apitk
cmake -S . -B build -G Ninja \
  -DITK_DIR=$DEPS/itk/build \
  -DVTK_DIR=$DEPS/vtk/install/lib/cmake/vtk-9.3 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build -t ITK-SNAP
```

### Launching with `apitk`

Create a small wrapper script on your `PATH`, for example `~/.local/bin/apitk`:

```bash
#!/bin/sh
exec /path/to/apitk/build/ITK-SNAP --console "$@"
```

```bash
chmod +x ~/.local/bin/apitk
```

Or add an alias to `~/.bashrc`: `alias apitk='/path/to/apitk/build/ITK-SNAP --console'`.
After that, typing `apitk` (optionally followed by image files) opens the
fork. Any installed ITK-SNAP and its `itksnap` command keep working as before.

---

## Building on macOS (Apple Silicon)

These steps have been tested on an arm64 Mac running macOS 26 with the Command
Line Tools only (no full Xcode).

### 1. Tools and libraries

```bash
brew install cmake ninja libssh curl qt
```

Homebrew installs CMake 4.x, which refuses to configure older projects. Set
this before each of the VTK, ITK and apitk configures:

```bash
export CMAKE_POLICY_VERSION_MINIMUM=3.5
```

### 2. VTK 9.3.1, with three source patches

```bash
DEPS=$HOME/apitk-deps && mkdir -p $DEPS && cd $DEPS
git clone --recursive -b v9.3.1 https://github.com/Kitware/VTK.git vtk
```

VTK 9.3.1 does not build against the macOS 26 SDK and the current clang
without three small edits:

| File (under `vtk/`) | Change | Error it fixes |
|---|---|---|
| `ThirdParty/zlib/vtkzlib/zutil.h` | `#if defined(MACOS) \|\| defined(TARGET_OS_MAC)` → `#if defined(MACOS)` | `fdopen` gets redefined, which causes errors in `_stdio.h` |
| `ThirdParty/png/vtkpng/pngpriv.h` | remove `\|\| defined(TARGET_OS_MAC)` from the classic-Mac condition | `'fp.h' file not found` |
| `Utilities/octree/octree/octree_node.txx` (line ~213) | `this->_M_chilren[child]` → `this->m_children[child]` | a typo that new clang no longer accepts |

Then configure and install:

```bash
cmake -S vtk -B vtk/build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$DEPS/vtk/install \
  -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF \
  -DBUILD_SHARED_LIBS=OFF \
  -DVTK_GROUP_ENABLE_Qt=YES -DVTK_QT_VERSION=6 \
  -DVTK_MODULE_ENABLE_VTK_GUISupportQtQuick=NO \
  -DVTK_MODULE_ENABLE_VTK_GUISupportQtSQL=NO \
  -DVTK_MODULE_ENABLE_VTK_RenderingExternal=WANT \
  -DVTK_REQUIRED_OBJCXX_FLAGS= \
  -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=12.6
cmake --build vtk/build -t install
```

### 3. ITK v5.4.0

```bash
git clone --recursive -b v5.4.0 https://github.com/InsightSoftwareConsortium/ITK.git itk
cmake -S itk -B itk/build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF \
  -DModule_MorphologicalContourInterpolation=ON \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=12.6
cmake --build itk/build
```

As on Linux, point `ITK_DIR` at `itk/build`, not at an install directory.

### 4. apitk

```bash
cd /path/to/apitk
cmake -S . -B build -G Ninja \
  -DITK_DIR=$DEPS/itk/build \
  -DVTK_DIR=$DEPS/vtk/install/lib/cmake/vtk-9.3 \
  -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt;/opt/homebrew/opt/curl;/opt/homebrew/opt/libssh" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=12.6
cmake --build build -t ITK-SNAP
```

The result is a plain native arm64 executable, `build/ITK-SNAP`, not a `.app`
bundle.

You may see linker warnings like *"building for macOS-12.6 but linking with
dylib built for 14.0"*. These are harmless for local use. To silence them,
use `-DCMAKE_OSX_DEPLOYMENT_TARGET=14.0` instead.

### 5. Launching with `apitk`

Create `/opt/homebrew/bin/apitk`. That directory is already on your `PATH`
and you can write to it without `sudo`.

```bash
#!/bin/sh
exec /path/to/apitk/build/ITK-SNAP --console "$@"
```

```bash
chmod +x /opt/homebrew/bin/apitk
```

This can sit next to a normal ITK-SNAP install (`/Applications/ITK-SNAP.app`
and its `itksnap` command) without replacing it. In the terminal, `apitk` runs
the fork and `itksnap` runs the official release.

---

## Windows

### Easiest: download a prebuilt ZIP

Every push to the `apitk` branch runs the **apitk build** GitHub Actions
workflow (`.github/workflows/apitk-build.yml`), which builds and tests apitk on
Windows, Linux and Apple Silicon macOS.

1. Open the repo's **Actions** tab, choose **apitk build**, then the latest
   green run.
2. Under **Artifacts**, download `apitk-windows-2022`. It contains
   `itksnap-4.4.0-…-win64-AMD64.zip`.
3. Unzip it anywhere, for example `C:\Tools\apitk`. No installer runs and
   nothing is registered, so it never conflicts with a regular ITK-SNAP
   install in `C:\Program Files\ITK-SNAP 4.4`.
4. Run `bin\ITK-SNAP.exe`, or use the `apitk` command:
   - Add the unzipped `bin` folder to your `PATH` (Settings → System → About →
     Advanced system settings → Environment Variables → `Path` → New).
   - Open a new terminal and type `apitk` (runs `bin\apitk.bat`, which
     launches `ITK-SNAP.exe --console`).

If it fails to start with a missing `VCRUNTIME140.dll` or `MSVCP140.dll`, run
the `vc_redist.x64.exe` that ships in the same `bin` folder.

The Linux (`apitk-ubuntu-22.04`, a `.tar.gz`) and macOS (`apitk-macos-14`, an
unsigned `.dmg`) artifacts work the same way. On macOS, right-click → Open the
first time, because the app isn't code-signed. Each run also uploads a
`ctest-log-<os>` artifact with the test output.

> Artifacts are kept for 90 days. To build again without pushing, use
> **Run workflow** on the **apitk build** page.

### Building from source on Windows

This mirrors the CI job and hasn't been tried outside it. You need:

- Visual Studio 2022 (Desktop C++ workload), CMake ≥ 3.16, Ninja, Git
- Qt 6 for `msvc2022_64` (Qt online installer, or `aqt install-qt windows desktop 6.8.1 win64_msvc2022_64`)
- [vcpkg](https://github.com/microsoft/vcpkg): `vcpkg install curl libssh zlib --triplet x64-windows-release`

Run everything from an **x64 Native Tools Command Prompt for VS 2022**:

```bat
:: VTK 9.3.1 (static)
cmake -S vtk -B vtk\build -G Ninja -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH=C:\Qt\6.8.1\msvc2022_64 ^
  -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=OFF ^
  -DVTK_GROUP_ENABLE_Qt=YES -DVTK_MODULE_ENABLE_VTK_GUISupportQtQuick=NO ^
  -DVTK_MODULE_ENABLE_VTK_GUISupportQtSQL=NO -DVTK_MODULE_ENABLE_VTK_RenderingExternal=WANT ^
  -DVTK_SMP_ENABLE_STDTHREAD=OFF -DCMAKE_INSTALL_PREFIX=%CD%\vtk\install
cmake --build vtk\build -t install

:: ITK v5.4.0 (/FORCE:MULTIPLE works around an ITK 5.4.0 link issue)
cmake -S itk -B itk\build -G Ninja -DCMAKE_BUILD_TYPE=Release ^
  -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF -DModule_MorphologicalContourInterpolation=ON ^
  -DCMAKE_EXE_LINKER_FLAGS=/FORCE:MULTIPLE
cmake --build itk\build

:: apitk
cmake -S apitk -B apitk\build -G Ninja -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=<vcpkg>\scripts\buildsystems\vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x64-windows-release ^
  -DCMAKE_PREFIX_PATH=C:\Qt\6.8.1\msvc2022_64 ^
  -DITK_DIR=%CD%\itk\build -DVTK_DIR=%CD%\vtk\install\lib\cmake\vtk-9.3 ^
  -DCMAKE_EXE_LINKER_FLAGS=/FORCE:MULTIPLE
cmake --build apitk\build
cd apitk\build && cpack -G ZIP
```

`cpack -G ZIP` produces the same portable ZIP as CI, with `ITK-SNAP.exe`, the
Qt DLLs and `apitk.bat` in its `bin` folder. Plain `cpack` builds upstream's
NSIS installer instead, which installs into `C:\Program Files\ITK-SNAP 4.4`
and *would* collide with an official ITK-SNAP 4.4 install. Avoid it if you
have one.

---

## Upstream ITK-SNAP README

The rest of this file is the original ITK-SNAP README. Questions about
ITK-SNAP in general belong upstream. Questions about Label Paint belong in
this fork's GitHub issues.

### Welcome!

Thank you for your interest in ITK-SNAP! By using this program you are joining
an ever growing community of users from research labs around the world.
ITK-SNAP is an open-source software application made possible by the
committment of dedicated developers, who often make contributions on a
volunteer basis. Original development of ITK-SNAP was supported by the U.S.
National Library of Medicine through PO 467-MZ-202446-1. Subsequent
development of ITK-SNAP version 2.0 has been supported by the U.S. National
Institute of Biomedical Imaging and BioEngineering and the NIH Blueprint for
Neuroscience through grant 1 R03 EB008200-01. Development of ITK-SNAP versions
3.0 - 3.6 has been supported by the NIH grant 1R01 EB014346. 

ITK-SNAP is the result of over 20 years of work by many researchers, software
engineers and students. Please visit the [Credits page](http://itksnap.org/credits.php) 
to learn about the team that made ITK-SNAP possible!

### Please remember to cite ITK-SNAP

Citations in published work is the best way for us to gauge ITK-SNAP impact.
As of the Summer of 2024, ITK-SNAP has been cited by [over 8000 papers](https://goo.gl/YoM9EK). 
If you use ITK-SNAP (including this fork) for a publication, please cite the paper below.

* Paul A. Yushkevich, Joseph Piven, Heather Cody Hazlett, Rachel Gimpel Smith,
  Sean Ho, James C. Gee, and Guido Gerig. User-guided 3D active contour
  segmentation of anatomical structures: Significantly improved efficiency and
  reliability. Neuroimage. 2006 Jul 1; 31(3):1116-28. 

### Getting Started 

*   [Online tutorials](http://www.itksnap.org/pmwiki/pmwiki.php?n=Documentation.HomePage)
    for using ITK-SNAP. Everything in them also applies to apitk.
*   Upstream [build instructions](http://www.itksnap.org/pmwiki/pmwiki.php?n=Documentation.BuildingITK-SNAP).
    For this fork, use the instructions above instead.
*   Sample images are on the [ITK-SNAP download page](http://www.itksnap.org/download/snap).
*   Training course materials (images, exercises, video) are on the
    [documentation page](http://www.itksnap.org/pmwiki/pmwiki.php?n=Documentation.SNAP3). 

### Asking Questions

*   Questions about using ITK-SNAP: [ITK-SNAP Users' List](http://www.itksnap.org/pmwiki/pmwiki.php?n=MailingLists).
*   Technical or ITK questions: [ITK Users' Mailing List](http://www.itk.org/HTML/MailingLists.htm).

### Get Involved!

*   [ITK-SNAP User and Developer Mailing Lists](http://www.itksnap.org/pmwiki/pmwiki.php?n=MailingLists).
*   Report upstream bugs on the [Bug Tracker](http://www.itksnap.org/pmwiki/pmwiki.php?n=Main.BugTracker).
*   Upstream development: [SourceForge.net](http://sourceforge.net/projects/itk-snap) and
    [GitHub](https://github.com/pyushkevich/itksnap).
