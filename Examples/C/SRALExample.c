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
	SRAL_RegisterKeyboardHooks();
	// Speak some text
	bool result = *(bool*)SRAL_GetEngineParameter(ENGINE_NVDA, NVDA_IS_CONTROL_EX);
	printf("NVDA extended: %d\n", result);
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

	// Set voice
	if (SRAL_GetEngineFeatures(0) & SUPPORTS_SELECT_VOICE) {
		int voice_count = *(int*)SRAL_GetEngineParameter(0, VOICE_COUNT);
		if (voice_count > 0) {
			const char** voices = (const char**)SRAL_GetEngineParameter(0, VOICE_LIST);
			if (voices != NULL) {
				printf("Enter voice index to select\nVoice list:\n");
				for (int i = 0; i < voice_count; i++) {
					printf("%d: %s\n", i, voices[i]);
					free((void*)voices[i]);
				}
				free(voices);
				int index = 0;
				scanf("%d", &index);
				if (index >= 0 && index < voice_count) {
					SRAL_SetEngineParameter(0, VOICE_INDEX, &index);
				}
			}
		}
	}

	// Speech rate
	if (SRAL_GetEngineFeatures(0) & SUPPORTS_SPEECH_RATE) {

		int rate = *(int*)SRAL_GetEngineParameter(ENGINE_NONE, SPEECH_RATE);

		const uint64_t max_rate = rate + 10;
		for (rate; rate < max_rate; rate++) {
			SRAL_SetEngineParameter(ENGINE_NONE, SPEECH_RATE, &rate);
			SRAL_Speak(text, false);
			sleep_ms(500);
		}
	}
	// Uninitialize the SRAL library
	SRAL_Uninitialize();

	return 0;
}
