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
To build on Linux you need to install libspeechd-dev, libx11-dev and brltty


## Support for NVDAControlEx

SRAL supports the [NVDAControlEx](https://github.com/m1maker/NVDAControlEx) add-on, allowing developers to extended manage the NVDA functions.

## Usage

To use the SRAL API in a C/C++ project, you need a statically linked or dynamically imported SRAL library, as well as a SRAL.h file with function declarations.
If you use SRAL as a static library for Windows, you need to define SRAL_STATIC in the SRAL.h before the include
```
#define SRAL_STATIC
#include <SRAL.h>
```

## Bindings
SRAL also has Bindings, which is updated with new wrappers for programming languages.

## Example
## C
```
#include <stdbool.h>
#include <stdio.h>
#define SRAL_STATIC
#include <SRAL.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void sleep_ms(int milliseconds) {
#ifdef _WIN32
	Sleep(milliseconds); // Windows-specific function
#else
	usleep(milliseconds * 1000); // usleep takes microseconds
#endif
}

int main(void) {
	char text[10000];
	// Initialize the SRAL library
	if (!SRAL_Initialize(SRAL_ENGINE_UIA)) { // So Microsoft UIAutomation provider can't speak in the terminal or none current process windows
		printf("Failed to initialize SRAL library.\n");
		return 1;
	}
	SRAL_RegisterKeyboardHooks();
	// Speak some text
	if (SRAL_GetEngineFeatures(0) & SUPPORTS_SPEECH) {
		printf("Enter the text you want to be spoken:\n");
		scanf("%s", text);
		SRAL_Speak(text, false);
	}

	// Output text to a Braille display
	if (SRAL_GetEngineFeatures(0) & SUPPORTS_BRAILLE) {
		printf("Enter the text you want to be shown on braille display:\n");
		scanf("%s", text);
		SRAL_Braille(text);

	}

	// Delay example
	SRAL_Output("Delay example: Enter any text", false);
	SRAL_Delay(5000);
	SRAL_Output("Press enter to continue", false);
	scanf("%s", text);

	SRAL_StopSpeech(); // Stops the delay thread
	// Uninitialize the SRAL library
	SRAL_Uninitialize();

	return 0;
}

```


## Additional info
For [NVDA](https://github.com/nvaccess/nvda) screen reader, you need to download the [Controller Client](https://www.nvaccess.org/files/nvda/releases/stable/). We don't support old client V 1.

