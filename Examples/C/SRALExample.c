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
	if (!SRAL_Initialize(ENGINE_UIA)) { // So Microsoft UIAutomation provider can't speak in the terminal or none current process windows
		printf("Failed to initialize SRAL library.\n");
		return 1;
	}

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

	// Speech rate
	if (SRAL_GetEngineFeatures(0) & SUPPORTS_SPEECH_RATE) {

		uint64_t rate = SRAL_GetRate();
		const uint64_t max_rate = rate + 10;
		for (rate; rate < max_rate; rate++) {
			SRAL_SetRate(rate);
			SRAL_Speak(text, false);
			sleep_ms(500);
		}
	}
	// Uninitialize the SRAL library
	SRAL_Uninitialize();

	return 0;
}
