# SRAL
Screen Reader Abstraction Library
## Description
SRAL is a cross-platform library for output text using speech engines.

## Platforms
SRAL is supported on Windows, MacOS and Linux platforms.

## Header
See how to use SRAL in Include/SRAL.h

## Compilation
SRAL can build using CMake into two libraries, static and dynamic.
Run these commands
```
cmake . -B build
cmake --build build --config Release
```

You will also have an executable test to test the SRAL.


# Warning
To build on Linux you need to install libspeechd-dev, libbrlapi-dev and brltty


## Support for NVDAControlEx

SRAL supports the [NVDAControlEx](https://github.com/m1maker/NVDAControlEx) add-on, allowing developers to extended manage the NVDA functions.

## Usage

To use the SRAL API in a C/C++ project, you need a statically linked or dynamically imported SRAL library, as well as a SRAL.h file with function declarations.
If you use SRAL as a static library for Windows, you need to define SRAL_STATIC in the SRAL.h before the include
```
#define SRAL_STATIC
#include <SRAL.h>
```

## Additional info
For [NVDA](https://github.com/nvaccess/nvda) screen reader, you need to download the [Controller Client](https://www.nvaccess.org/files/nvda/releases/stable/). We don't support old client V 1.

