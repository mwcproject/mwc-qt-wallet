Building mwc-qt-wallet for Windows
==================================

Development of mwc-qt-wallet in Windows environments can be achieved easily using either the Qt Creator IDE or
Visual Studio versions from 2015 onward. This guide will focus primarily on the latter, though tips for setting
up a working Qt Creator environment can be found in the [Qt Creator](#qt-creator) section of this document.

## Prerequisites

* [Visual Studio 2015 or newer](https://visualstudio.microsoft.com/downloads/)
  * C++ CMake tools for Windows
  * Clang compiler for Windows (Optional - enables Clang builds in Visual Studio 2019)
  * MSBuild
  * MSVC v140 - VS 2015 C++ build tools (v14.00)
* [Qt 5.9.8](https://www.qt.io/download)
  * MSVC 2015 64-bit
* [Windows 8.1 SDK or newer](https://developer.microsoft.com/en-us/windows/downloads/sdk-archive)
  * Windows Software Development Kit
  * Debugging Tools for Windows (Optional - enables debugging with Qt Creator)
* [mwc713 v2.0.2-beta.1 or newer](https://github.com/cgilliard/mwc713/releases)

## Preparing your IDE

### Visual Studio 2019 with Ninja

1. Edit CMakeLists.txt in the mwc-qt-wallet root so that Qt_DIR points to the directory containing
   your installed Qt libraries.
2. In Visual Studio 2019, select the File menu -> Open -> CMake.
3. Browse to the mwc-qt-wallet root and select `CMakeLists.txt`.

**NOTES**

* Upon opening `CMakeLists.txt` for the first time, the CMake cache will be generated and a file
  `CMakeSettings.json` will be created, containing project configuration information used by Visual Studio.
* If you installed the Clang compiler for Windows, you may create Clang build configurations in
  CMakeSettings.json.
* The Project menu contains items for tasks like deleting or regenerating the CMake cache.
* The Solution Explorer offers a handy "CMake Targets View" to present the project neatly.
* Builds (and the CMake cache) will be placed  in the `out` directory under the mwc-qt-wallet root.
* Please also see the [General Notes](#general-notes) section of this document.

### Visual Studio 2015/2017/2019 with MSBuild

1. Edit CMakeLists.txt in the mwc-qt-wallet root so that Qt_DIR points to the directory containing
   your installed Qt libraries.
2. Open the Developer Command Prompt for your Visual Studio version and desired target architecture.
   * Or open your favorite shell and run your the appropriate vcvarsXXX.bat.
3. Change directory to the mwc-qt-wallet root.
4. Run `gen_vs_sln.bat`.
   * Follow the prompts, resolve any errors, and run again if necessary.
   * A new directory, `VC\<arch>`, will be created in the mwc-qt-wallet root containing the CMake cache.
     and the generated Visual Studio solution and project files.

**NOTES**

* `gen_vs_sln.bat` will generate project files for your current target architecture. If the wrong architecture
  is generated, please ensure you have launched gen_vs_sln.bat from the correct developer prompt.
* Builds will be found in the `VC\<arch>` directory under the mwc-qt-wallet root.

### Qt Creator with QMake

**TODO: this.**

### General Notes

* mwc-qt-wallet requires mwc713 to run. Place the mwc713 executable in your build output directory.
* Certain required Qt libraries will be copied into your build output directory at the end of a successful build.
  These must be included with the distributed package. Corresponding PDB files for debugging may be found in your Qt
  installation directory.
* The Visual C++ Runtime files are required on the end-user machine. This requirement may be satisfied by including
  either of the following with the distributed package:
  * [Visual C++ Redistributable for Visual Studio 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48145)
  * `concrt140.dll`, `msvcp140.dll`, and `vcruntime140.dll` from the Visual Studio 2015 redist directory.
* Newer or older versions of Qt, Visual Studio, and/or the Windows SDK than those mentioned in this document may work,
  but they have not been tested and support will not be offered for them.
