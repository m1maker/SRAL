#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For strcmp, strcpy

// Define SRAL_STATIC if linking against a static SRAL library
#define SRAL_STATIC
#include <SRAL.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h> // For usleep
#endif


void sleep_ms(int milliseconds) {
#ifdef _WIN32
	Sleep(milliseconds);
#else
	usleep((useconds_t)milliseconds * 1000);
#endif
}

void prompt_user(const char* message) {
	printf("\n>>> %s (Press Enter to continue)...", message);
	int c;
	while ((c = getchar()) != '\n' && c != EOF);

	if (c == EOF && feof(stdin)) {
		printf("EOF detected on stdin, continuing without prompt.\n");
	}
}


void PrintEngineNames(int engineBitmask, const char* title) {
	printf("%s:\n", title);
	if (engineBitmask == ENGINE_NONE) {
		printf("  (None)\n");
		return;
	}
	bool found = false;
	for (int engine_val = ENGINE_NVDA; engine_val <= ENGINE_NARRATOR; engine_val <<= 1) {
		if (engineBitmask & engine_val) {
			const char* name = SRAL_GetEngineName(engine_val);
			printf("  - %s (0x%X)\n", name ? name : "Unknown Engine", engine_val);
			found = true;
		}
	}
	if (!found && engineBitmask != 0) {
		printf("  (Unknown bitmask: 0x%X)\n", engineBitmask);
	}
	printf("\n");
}

void print_supported_features(int features) {
	printf("Supported Features (0x%X):\n", features);
	if (features == 0) {
		printf("  (None)\n");
		return;
	}
	if (features & SUPPORTS_SPEECH) printf("  - SUPPORTS_SPEECH\n");
	if (features & SUPPORTS_BRAILLE) printf("  - SUPPORTS_BRAILLE\n");
	if (features & SUPPORTS_SPEECH_RATE) printf("  - SUPPORTS_SPEECH_RATE\n");
	if (features & SUPPORTS_SPEECH_VOLUME) printf("  - SUPPORTS_SPEECH_VOLUME\n");
	if (features & SUPPORTS_SELECT_VOICE) printf("  - SUPPORTS_SELECT_VOICE\n");
	if (features & SUPPORTS_PAUSE_SPEECH) printf("  - SUPPORTS_PAUSE_SPEECH\n");
	if (features & SUPPORTS_SSML) printf("  - SUPPORTS_SSML\n");
	if (features & SUPPORTS_SPEAK_TO_MEMORY) printf("  - SUPPORTS_SPEAK_TO_MEMORY\n");
	if (features & SUPPORTS_SPELLING) printf("  - SUPPORTS_SPELLING\n");
	printf("\n");
}

#define TEST_SECTION(name) \
    printf("\n\n========================================\n"); \
    printf("  Testing: %s\n", name); \
    printf("========================================\n");

#define CHECK(condition, success_msg, fail_msg) \
    if (condition) { \
        printf("[SUCCESS] %s\n", success_msg); \
    } else { \
        printf("[FAILURE] %s\n", fail_msg); \
    }

#define CHECK_SRAL(func_call, action_desc) \
    { \
        bool success_val = func_call; \
        if (success_val) { \
            printf("[SUCCESS] %s\n", action_desc); \
        } else { \
            printf("[FAILURE] %s\n", action_desc); \
        } \
    }

int main(void) {
	printf("SRAL Tester\n");
	printf("-------------------------\n");

	TEST_SECTION("SRAL_IsInitialized (Before Initialization)");
	CHECK(!SRAL_IsInitialized(), "SRAL_IsInitialized correctly returns false before init.", "SRAL_IsInitialized returned true before init!");

	TEST_SECTION("SRAL_Initialize");
	int engines_to_exclude = ENGINE_NONE;
	engines_to_exclude = ENGINE_UIA;
	printf("Attempting to initialize SRAL, excluding engines: 0x%X (%s)\n",
		engines_to_exclude, SRAL_GetEngineName(engines_to_exclude) ? SRAL_GetEngineName(engines_to_exclude) : "None");

	if (SRAL_Initialize(engines_to_exclude)) {
		printf("[SUCCESS] SRAL_Initialize successful.\n");
	}
	else {
		printf("[FAILURE] SRAL_Initialize failed. Some tests may not run or behave as expected. Exiting.\n");
		return 1;
	}
	CHECK(SRAL_IsInitialized(), "SRAL_IsInitialized correctly returns true after init.", "SRAL_IsInitialized returned false after init!");

	TEST_SECTION("Engine Information");
	int available_engines = SRAL_GetAvailableEngines();
	PrintEngineNames(available_engines, "Available Engines on this Platform");

	int active_engines = SRAL_GetActiveEngines();
	PrintEngineNames(active_engines, "Currently Active/Usable Engines");

	int current_engine_id = SRAL_GetCurrentEngine();
	printf("Current Default Engine: %s (0x%X)\n", SRAL_GetEngineName(current_engine_id) ? SRAL_GetEngineName(current_engine_id) : "None/Unknown", current_engine_id);

	printf("\nNames of all SRAL_Engines enum members (as per SRAL_GetEngineName):\n");
	for (int e_val = ENGINE_NVDA; e_val <= ENGINE_NARRATOR; e_val <<= 1) {
		const char* name = SRAL_GetEngineName(e_val);
		printf("  Engine ID 0x%X: %s\n", e_val, name ? name : "(Name not defined or not a single engine ID)");
	}


	int specific_engine_for_ex_tests = ENGINE_NONE;
	if (active_engines != ENGINE_NONE) {
		for (int e_val = ENGINE_NVDA; e_val <= ENGINE_NARRATOR; e_val <<= 1) {
			if ((active_engines & e_val) && e_val != current_engine_id) {
				specific_engine_for_ex_tests = e_val;
				break;
			}
		}
		if (specific_engine_for_ex_tests == ENGINE_NONE) {
			for (int e_val = ENGINE_NVDA; e_val <= ENGINE_NARRATOR; e_val <<= 1) {
				if (active_engines & e_val) {
					specific_engine_for_ex_tests = e_val;
					break;
				}
			}
		}
	}

	if (specific_engine_for_ex_tests != ENGINE_NONE) {
		printf("\nWill use engine '%s' (0x%X) for specific engine (Ex) tests.\n",
			SRAL_GetEngineName(specific_engine_for_ex_tests), specific_engine_for_ex_tests);
	}
	else {
		printf("\nNo specific engine distinct from default (or no active engines) for Ex tests. Ex tests might target default or fail if engine doesn't support action.\n");
	}


	TEST_SECTION("Keyboard Hooks");
	if (SRAL_RegisterKeyboardHooks()) {
		printf("[SUCCESS] SRAL_RegisterKeyboardHooks registered.\n");
		prompt_user("Keyboard hooks (Ctrl=Interrupt, Shift=Pause/Resume) are active. Test them with upcoming speech. This primarily affects non-screen-reader engines like SAPI or SpeechDispatcher.");
	}
	else {
		printf("[INFO] SRAL_RegisterKeyboardHooks failed or did not register. This might be expected if no suitable engine is active or if permissions are lacking.\n");
	}

	TEST_SECTION("SRAL_GetEngineFeatures");
	printf("Features for Current Default Engine (%s):\n", SRAL_GetEngineName(current_engine_id) ? SRAL_GetEngineName(current_engine_id) : "None");
	int current_engine_features = SRAL_GetEngineFeatures(ENGINE_NONE);
	print_supported_features(current_engine_features);

	if (specific_engine_for_ex_tests != ENGINE_NONE) {
		printf("Features for Specific Engine selected for Ex tests (%s):\n", SRAL_GetEngineName(specific_engine_for_ex_tests));
		int specific_engine_features = SRAL_GetEngineFeatures(specific_engine_for_ex_tests);
		print_supported_features(specific_engine_features);
	}

	if (current_engine_features & SUPPORTS_SPEECH) {
		TEST_SECTION("SRAL_Speak (Default Engine)");
		CHECK_SRAL(SRAL_Speak("Testing SRAL Speak, not interrupting previous speech.", false), "SRAL_Speak (no interrupt)");
		sleep_ms(2000);
		CHECK_SRAL(SRAL_Speak("Testing SRAL Speak, interrupting previous speech.", true), "SRAL_Speak (interrupt)");
		sleep_ms(2000);

		if (specific_engine_for_ex_tests != ENGINE_NONE) {
			TEST_SECTION("SRAL_SpeakEx (Specific Engine)");
			int features_ex = SRAL_GetEngineFeatures(specific_engine_for_ex_tests);
			if (features_ex & SUPPORTS_SPEECH) {
				CHECK_SRAL(SRAL_SpeakEx(specific_engine_for_ex_tests, "Testing SRAL SpeakEx, not interrupting.", false), "SRAL_SpeakEx (no interrupt)");
				sleep_ms(2000);
				CHECK_SRAL(SRAL_SpeakEx(specific_engine_for_ex_tests, "Testing SRAL SpeakEx, interrupting.", true), "SRAL_SpeakEx (interrupt)");
				sleep_ms(2000);
			}
			else {
				printf("Specific engine %s does not support speech for SpeakEx.\n", SRAL_GetEngineName(specific_engine_for_ex_tests));
			}
		}
	}
	else {
		printf("Current default engine does not support speech. Skipping speech tests.\n");
	}

	if (current_engine_features & SUPPORTS_SSML) {
		TEST_SECTION("SRAL_SpeakSsml (Default Engine)");
		const char* ssml_test = "<speak>This is SSML text.</speak>";
		CHECK_SRAL(SRAL_SpeakSsml(ssml_test, true), "SRAL_SpeakSsml");
		sleep_ms(3000);

		if (specific_engine_for_ex_tests != ENGINE_NONE) {
			TEST_SECTION("SRAL_SpeakSsmlEx (Specific Engine)");
			int features_ex = SRAL_GetEngineFeatures(specific_engine_for_ex_tests);
			if (features_ex & SUPPORTS_SSML) {
				CHECK_SRAL(SRAL_SpeakSsmlEx(specific_engine_for_ex_tests, ssml_test, true), "SRAL_SpeakSsmlEx");
				sleep_ms(3000);
			}
			else {
				printf("Specific engine %s does not support SSML for SpeakSsmlEx.\n", SRAL_GetEngineName(specific_engine_for_ex_tests));
			}
		}
	}
	else {
		printf("Current default engine does not support SSML. Skipping SSML tests.\n");
	}


	if (current_engine_features & SUPPORTS_SPEAK_TO_MEMORY) {
		TEST_SECTION("SRAL_SpeakToMemory (Default Engine)");
		uint64_t buffer_size;
		int channels, sample_rate, bits_per_sample;
		void* pcm_buffer = SRAL_SpeakToMemory("Testing speak to memory audio synthesis.", &buffer_size, &channels, &sample_rate, &bits_per_sample);
		if (pcm_buffer) {
			printf("[SUCCESS] SRAL_SpeakToMemory successful.\n");
			printf("  Buffer Size: %llu bytes\n", (unsigned long long)buffer_size);
			printf("  Channels: %d\n", channels);
			printf("  Sample Rate: %d Hz\n", sample_rate);
			printf("  Bits Per Sample: %d\n", bits_per_sample);
			free(pcm_buffer); // SRAL user is responsible for freeing this memory
			printf("  PCM buffer freed.\n");
		}
		else {
			printf("[FAILURE] SRAL_SpeakToMemory failed.\n");
		}

		if (specific_engine_for_ex_tests != ENGINE_NONE) {
			TEST_SECTION("SRAL_SpeakToMemoryEx (Specific Engine)");
			int features_ex = SRAL_GetEngineFeatures(specific_engine_for_ex_tests);
			if (features_ex & SUPPORTS_SPEAK_TO_MEMORY) {
				pcm_buffer = SRAL_SpeakToMemoryEx(specific_engine_for_ex_tests, "Testing speak to memory ex.", &buffer_size, &channels, &sample_rate, &bits_per_sample);
				if (pcm_buffer) {
					printf("[SUCCESS] SRAL_SpeakToMemoryEx successful for engine %s.\n", SRAL_GetEngineName(specific_engine_for_ex_tests));
					printf("  Buffer Size: %llu bytes\n", (unsigned long long)buffer_size);
					printf("  Channels: %d\n", channels);
					printf("  Sample Rate: %d Hz\n", sample_rate);
					printf("  Bits Per Sample: %d\n", bits_per_sample);
					free(pcm_buffer);
					printf("  PCM buffer freed.\n");
				}
				else {
					printf("[FAILURE] SRAL_SpeakToMemoryEx failed for engine %s.\n", SRAL_GetEngineName(specific_engine_for_ex_tests));
				}
			}
			else {
				printf("Specific engine %s does not support Speak To Memory for SpeakToMemoryEx.\n", SRAL_GetEngineName(specific_engine_for_ex_tests));
			}
		}
	}
	else {
		printf("Current default engine does not support Speak To Memory. Skipping these tests.\n");
	}


	if (current_engine_features & SUPPORTS_BRAILLE) {
		TEST_SECTION("SRAL_Braille (Default Engine)");
		prompt_user("Prepare to check Braille display for 'Testing SRAL Braille output.'");
		CHECK_SRAL(SRAL_Braille("Testing SRAL Braille output."), "SRAL_Braille");

		if (specific_engine_for_ex_tests != ENGINE_NONE) {
			TEST_SECTION("SRAL_BrailleEx (Specific Engine)");
			int features_ex = SRAL_GetEngineFeatures(specific_engine_for_ex_tests);
			if (features_ex & SUPPORTS_BRAILLE) {
				prompt_user("Prepare to check Braille display for 'Testing SRAL Braille Ex output.'");
				CHECK_SRAL(SRAL_BrailleEx(specific_engine_for_ex_tests, "Testing SRAL Braille Ex output."), "SRAL_BrailleEx");
			}
			else {
				printf("Specific engine %s does not support Braille for BrailleEx.\n", SRAL_GetEngineName(specific_engine_for_ex_tests));
			}
		}
	}
	else {
		printf("Current default engine does not support Braille. Skipping Braille tests.\n");
	}

	TEST_SECTION("SRAL_Output (Default Engine)");
	prompt_user("Prepare for SRAL_Output (Speech and/or Braille) for 'Testing SRAL Output, not interrupting.'");
	CHECK_SRAL(SRAL_Output("Testing SRAL Output, not interrupting.", false), "SRAL_Output (no interrupt)");
	sleep_ms(2000);
	prompt_user("Prepare for SRAL_Output (Speech and/or Braille) for 'Testing SRAL Output, interrupting.'");
	CHECK_SRAL(SRAL_Output("Testing SRAL Output, interrupting now.", true), "SRAL_Output (interrupt)");
	sleep_ms(2000);

	if (specific_engine_for_ex_tests != ENGINE_NONE) {
		TEST_SECTION("SRAL_OutputEx (Specific Engine)");
		prompt_user("Prepare for SRAL_OutputEx with specific engine for 'Testing SRAL OutputEx, not interrupting.'");
		CHECK_SRAL(SRAL_OutputEx(specific_engine_for_ex_tests, "Testing SRAL OutputEx, not interrupting.", false), "SRAL_OutputEx (no interrupt)");
		sleep_ms(2000);
		prompt_user("Prepare for SRAL_OutputEx with specific engine for 'Testing SRAL OutputEx, interrupting.'");
		CHECK_SRAL(SRAL_OutputEx(specific_engine_for_ex_tests, "Testing SRAL OutputEx, interrupting now.", true), "SRAL_OutputEx (interrupt)");
		sleep_ms(2000);
	}

	if (current_engine_features & SUPPORTS_SPEECH) {
		TEST_SECTION("Speech Control (Default Engine)");
		const char* long_speech = "This is a moderately long sentence designed to test the pause, resume, and stop functionality of the SRAL library effectively.";
		printf("Speaking long sentence with default engine: \"%s\"\n", long_speech);
		SRAL_Speak(long_speech, true);
		prompt_user("Speech started. It should be speaking now. Press Enter to attempt PAUSE (if supported).");

		if (current_engine_features & SUPPORTS_PAUSE_SPEECH) {
			CHECK_SRAL(SRAL_PauseSpeech(), "SRAL_PauseSpeech");
			prompt_user("Speech Paused (hopefully). Press Enter to attempt RESUME.");
			CHECK_SRAL(SRAL_ResumeSpeech(), "SRAL_ResumeSpeech");
			prompt_user("Speech Resumed (hopefully). Press Enter to STOP.");
		}
		else {
			printf("Pause/Resume not supported by current default engine according to features. Will attempt stop directly.\n");
			prompt_user("Speech should be ongoing. Press Enter to STOP.");
		}
		CHECK_SRAL(SRAL_StopSpeech(), "SRAL_StopSpeech");
		printf("Speech should be stopped now.\n");
		sleep_ms(500);

		if (specific_engine_for_ex_tests != ENGINE_NONE) {
			TEST_SECTION("Speech Control Ex (Specific Engine)");
			int features_ex = SRAL_GetEngineFeatures(specific_engine_for_ex_tests);
			if (features_ex & SUPPORTS_SPEECH) {
				printf("Speaking long sentence with engine %s: \"%s\"\n", SRAL_GetEngineName(specific_engine_for_ex_tests), long_speech);
				SRAL_SpeakEx(specific_engine_for_ex_tests, long_speech, true);
				prompt_user("Speech started (Ex). Press Enter to PAUSE (Ex) (if supported).");

				if (features_ex & SUPPORTS_PAUSE_SPEECH) {
					CHECK_SRAL(SRAL_PauseSpeechEx(specific_engine_for_ex_tests), "SRAL_PauseSpeechEx");
					prompt_user("Speech Paused (Ex). Press Enter to RESUME (Ex).");
					CHECK_SRAL(SRAL_ResumeSpeechEx(specific_engine_for_ex_tests), "SRAL_ResumeSpeechEx");
					prompt_user("Speech Resumed (Ex). Press Enter to STOP (Ex).");
				}
				else {
					printf("Pause/Resume not supported by specific engine %s. Will attempt stop directly.\n", SRAL_GetEngineName(specific_engine_for_ex_tests));
					prompt_user("Speech should be ongoing (Ex). Press Enter to STOP (Ex).");
				}
				CHECK_SRAL(SRAL_StopSpeechEx(specific_engine_for_ex_tests), "SRAL_StopSpeechEx");
				printf("Speech should be stopped (Ex).\n");
				sleep_ms(500);
			}
			else {
				printf("Specific engine %s does not support speech. Skipping Speech Control Ex tests.\n", SRAL_GetEngineName(specific_engine_for_ex_tests));
			}
		}
	}


	TEST_SECTION("SRAL Engine Parameters (Default Engine)");

	if (current_engine_features & SUPPORTS_SPEECH_RATE) {
		printf("\nTesting SPEECH_RATE (Default Engine):\n");
		int current_rate, original_rate;
		if (SRAL_GetEngineParameter(ENGINE_NONE, SPEECH_RATE, &original_rate)) {
			printf("  Original rate: %d\n", original_rate);
			current_rate = original_rate;
			int new_rate = (original_rate <= 90) ? (original_rate + 10) : (original_rate - 10);
			if (new_rate < 0) new_rate = 0; if (new_rate > 100) new_rate = 100; // Clamp

			printf("  Attempting to set rate to: %d\n", new_rate);
			if (SRAL_SetEngineParameter(ENGINE_NONE, SPEECH_RATE, &new_rate)) {
				int fetched_rate;
				SRAL_GetEngineParameter(ENGINE_NONE, SPEECH_RATE, &fetched_rate);
				printf("  New rate confirmed by get: %d\n", fetched_rate);
				CHECK(abs(fetched_rate - new_rate) <= 5, "Rate set and get matches (or is close)", "Rate set/get mismatch or significant difference");
				SRAL_Speak("Testing new speech rate setting.", true);
				sleep_ms(2500);
				SRAL_SetEngineParameter(ENGINE_NONE, SPEECH_RATE, &original_rate);
				printf("  Restored original rate to: %d\n", original_rate);
			}
			else {
				printf("  Failed to set SPEECH_RATE.\n");
			}
		}
		else {
			printf("  Failed to get initial SPEECH_RATE.\n");
		}
	}
	else {
		printf("\nSPEECH_RATE not supported by current default engine.\n");
	}

	if (current_engine_features & SUPPORTS_SPEECH_VOLUME) {
		printf("\nTesting SPEECH_VOLUME (Default Engine):\n");
		int current_volume, original_volume;
		if (SRAL_GetEngineParameter(ENGINE_NONE, SPEECH_VOLUME, &original_volume)) {
			printf("  Original volume: %d\n", original_volume);
			current_volume = original_volume;
			int new_volume = (original_volume <= 90) ? (original_volume + 10) : (original_volume - 10);
			if (new_volume < 0) new_volume = 0; if (new_volume > 100) new_volume = 100;

			printf("  Attempting to set volume to: %d\n", new_volume);
			if (SRAL_SetEngineParameter(ENGINE_NONE, SPEECH_VOLUME, &new_volume)) {
				int fetched_volume;
				SRAL_GetEngineParameter(ENGINE_NONE, SPEECH_VOLUME, &fetched_volume);
				printf("  New volume confirmed by get: %d\n", fetched_volume);
				CHECK(abs(fetched_volume - new_volume) <= 5, "Volume set and get matches (or is close)", "Volume set/get mismatch or significant difference");
				SRAL_Speak("Testing new speech volume setting.", true);
				sleep_ms(2500);
				SRAL_SetEngineParameter(ENGINE_NONE, SPEECH_VOLUME, &original_volume);
				printf("  Restored original volume to: %d\n", original_volume);
			}
			else {
				printf("  Failed to set SPEECH_VOLUME.\n");
			}
		}
		else {
			printf("  Failed to get initial SPEECH_VOLUME.\n");
		}
	}
	else {
		printf("\nSPEECH_VOLUME not supported by current default engine.\n");
	}

	if (current_engine_features & SUPPORTS_SELECT_VOICE) {
		printf("\nTesting VOICE parameters (Default Engine):\n");
		int voice_count = 0;
		if (SRAL_GetEngineParameter(ENGINE_NONE, VOICE_COUNT, &voice_count)) {
			printf("  Voice count: %d\n", voice_count);
			if (voice_count > 0) {
				char** voice_list_names = (char**)malloc((size_t)voice_count * sizeof(char*));
				if (!voice_list_names) {
					printf("  Failed to allocate memory for voice list pointers.\n");
				}
				else {
					bool alloc_ok = true;
					for (int i = 0; i < voice_count; ++i) {
						voice_list_names[i] = (char*)malloc(128 * sizeof(char));
						if (!voice_list_names[i]) {
							printf("  Failed to allocate memory for voice name %d.\n", i);
							for (int j = 0; j < i; ++j) free(voice_list_names[j]);
							free(voice_list_names);
							voice_list_names = NULL;
							alloc_ok = false;
							break;
						}
						voice_list_names[i][0] = '\0';
					}

					if (alloc_ok && SRAL_GetEngineParameter(ENGINE_NONE, VOICE_LIST, voice_list_names)) {
						printf("  Available voices:\n");
						for (int i = 0; i < voice_count; ++i) {
							printf("    %d: %s\n", i, voice_list_names[i] ? voice_list_names[i] : "(null string)");
						}

						int current_voice_index = -1, original_voice_index = -1;
						SRAL_GetEngineParameter(ENGINE_NONE, VOICE_INDEX, &original_voice_index);
						printf("  Current voice index: %d (%s)\n", original_voice_index, (original_voice_index >= 0 && original_voice_index < voice_count && voice_list_names[original_voice_index]) ? voice_list_names[original_voice_index] : "Unknown/Default");

						if (voice_count > 1) {
							int new_voice_index = (original_voice_index + 1) % voice_count;
							printf("  Attempting to set voice to index: %d (%s)\n", new_voice_index, voice_list_names[new_voice_index]);
							if (SRAL_SetEngineParameter(ENGINE_NONE, VOICE_INDEX, &new_voice_index)) {
								SRAL_GetEngineParameter(ENGINE_NONE, VOICE_INDEX, current_voice_index);
								printf("  New voice index confirmed by get: %d\n", current_voice_index);
								CHECK(current_voice_index == new_voice_index, "Voice index set and get matches", "Voice index set/get mismatch");
								SRAL_Speak("Testing newly selected voice.", true);
								sleep_ms(3000);
								if (original_voice_index != -1) {
									SRAL_SetEngineParameter(ENGINE_NONE, VOICE_INDEX, &original_voice_index);
									printf("  Restored original voice index to: %d\n", original_voice_index);
								}
							}
							else {
								printf("  Failed to set VOICE_INDEX.\n");
							}
						}
					}
					else if (alloc_ok) {
						printf("  Failed to get VOICE_LIST.\n");
					}

					if (voice_list_names) {
						for (int i = 0; i < voice_count; ++i) if (voice_list_names[i]) free(voice_list_names[i]);
						free(voice_list_names);
					}
				}
			}
		}
		else {
			printf("  Failed to get VOICE_COUNT.\n");
		}
	}
	else {
		printf("\nVOICE selection not supported by current default engine.\n");
	}

	printf("\nTesting ENABLE_SPELLING (Default Engine):\n");
	bool spelling_enabled, original_spelling_state;
	if (SRAL_GetEngineParameter(ENGINE_NONE, ENABLE_SPELLING, &original_spelling_state)) {
		printf("  Initial spelling state: %s\n", original_spelling_state ? "Enabled" : "Disabled");
		bool new_spelling_state = !original_spelling_state;
		printf("  Attempting to set spelling to: %s\n", new_spelling_state ? "Enabled" : "Disabled");
		if (SRAL_SetEngineParameter(ENGINE_NONE, ENABLE_SPELLING, &new_spelling_state)) {
			SRAL_GetEngineParameter(ENGINE_NONE, ENABLE_SPELLING, &spelling_enabled);
			printf("  New spelling state confirmed by get: %s\n", spelling_enabled ? "Enabled" : "Disabled");
			CHECK(spelling_enabled == new_spelling_state, "Spelling state set/get matches", "Spelling state set/get mismatch");
			SRAL_Speak(new_spelling_state ? "Spelling test: H E L L O" : "Spelling test: Hello", true);
			sleep_ms(3000);
			SRAL_SetEngineParameter(ENGINE_NONE, ENABLE_SPELLING, &original_spelling_state); // Restore
			printf("  Restored original spelling state.\n");
		}
		else {
			printf("  Failed to set ENABLE_SPELLING (This might be normal if not truly settable by this engine, or if feature isn't fully supported).\n");
		}
	}
	else {
		printf("  Failed to get initial ENABLE_SPELLING (This might be normal if not supported by current engine).\n");
	}

	// Test NVDA_IS_CONTROL_EX (Only if NVDA is active or selected)
	if (current_engine_id == ENGINE_NVDA || (specific_engine_for_ex_tests != ENGINE_NONE && specific_engine_for_ex_tests == ENGINE_NVDA)) {
		printf("\nTesting NVDA_IS_CONTROL_EX (for NVDA):\n");
		bool nvda_is_control_ex_val;
		if (SRAL_GetEngineParameter(ENGINE_NVDA, NVDA_IS_CONTROL_EX, &nvda_is_control_ex_val)) {
			printf("  NVDA_IS_CONTROL_EX value from NVDA engine: %s\n", nvda_is_control_ex_val ? "true (extended control available)" : "false (standard control)");
		}
		else {
			printf("  Failed to get NVDA_IS_CONTROL_EX (NVDA might not be the one responding or parameter not available).\n");
		}
	}

	if (current_engine_id == ENGINE_SAPI || (specific_engine_for_ex_tests != ENGINE_NONE && specific_engine_for_ex_tests == ENGINE_SAPI)) {
		printf("\nTesting SAPI_TRIM_THRESHOLD (for SAPI):\n");
		int trim_threshold, original_trim_threshold;
		if (SRAL_GetEngineParameter(ENGINE_SAPI, SAPI_TRIM_THRESHOLD, &original_trim_threshold)) {
			printf("  Initial SAPI_TRIM_THRESHOLD: %d\n", original_trim_threshold);
			int new_threshold = original_trim_threshold == 20 ? 60 : 20;
			printf("  Attempting to set SAPI_TRIM_THRESHOLD to: %d\n", new_threshold);
			if (SRAL_SetEngineParameter(ENGINE_SAPI, SAPI_TRIM_THRESHOLD, &new_threshold)) {
				SRAL_GetEngineParameter(ENGINE_SAPI, SAPI_TRIM_THRESHOLD, &trim_threshold);
				printf("  New SAPI_TRIM_THRESHOLD confirmed by get: %d\n", trim_threshold);
				CHECK(trim_threshold == new_threshold, "SAPI_TRIM_THRESHOLD set/get matches", "SAPI_TRIM_THRESHOLD set/get mismatch");
				SRAL_SpeakEx(ENGINE_SAPI, "Testing SAPI trim threshold. ... Some silence here.", true);
				sleep_ms(3000);
				SRAL_SetEngineParameter(ENGINE_SAPI, SAPI_TRIM_THRESHOLD, &original_trim_threshold);
				printf("  Restored SAPI_TRIM_THRESHOLD.\n");
			}
			else {
				printf("  Failed to set SAPI_TRIM_THRESHOLD.\n");
			}
		}
		else {
			printf("  Failed to get initial SAPI_TRIM_THRESHOLD (SAPI might not be responding or param not available).\n");
		}
	}

	printf("\nTesting SYMBOL_LEVEL (Default Engine):\n");
	int symbol_level, original_symbol_level;
	if (SRAL_GetEngineParameter(ENGINE_NONE, SYMBOL_LEVEL, &original_symbol_level)) {
		printf("  Original symbol_level: %d\n", original_symbol_level);
		int new_symbol_level = (original_symbol_level + 1) % 4;
		printf("  Attempting to set symbol_level to: %d\n", new_symbol_level);
		if (SRAL_SetEngineParameter(ENGINE_NONE, SYMBOL_LEVEL, &new_symbol_level)) {
			SRAL_GetEngineParameter(ENGINE_NONE, SYMBOL_LEVEL, &symbol_level);
			printf("  New symbol_level confirmed by get: %d\n", symbol_level);
			CHECK(symbol_level == new_symbol_level, "Symbol level set/get matches", "Symbol level set/get mismatch");
			SRAL_Speak("Testing symbol level with punctuation! At symbol @ hash # dollar $ percent % caret ^ ampersand & asterisk *.", true);
			sleep_ms(4000);
			SRAL_SetEngineParameter(ENGINE_NONE, SYMBOL_LEVEL, &original_symbol_level);
			printf("  Restored original symbol_level: %d\n", original_symbol_level);
		}
		else {
			printf("  Failed to set SYMBOL_LEVEL (might not be supported or settable by this engine).\n");
		}
	}
	else {
		printf("  Failed to get initial SYMBOL_LEVEL (might not be supported by current engine).\n");
	}

	printf("\nTesting USE_CHARACTER_DESCRIPTIONS (Default Engine):\n");
	bool use_char_desc, original_use_char_desc;
	if (SRAL_GetEngineParameter(ENGINE_NONE, USE_CHARACTER_DESCRIPTIONS, &original_use_char_desc)) {
		printf("  Initial USE_CHARACTER_DESCRIPTIONS: %s\n", original_use_char_desc ? "Enabled" : "Disabled");
		bool new_use_char_desc = !original_use_char_desc;
		printf("  Attempting to set USE_CHARACTER_DESCRIPTIONS to: %s\n", new_use_char_desc ? "Enabled" : "Disabled");
		if (SRAL_SetEngineParameter(ENGINE_NONE, USE_CHARACTER_DESCRIPTIONS, &new_use_char_desc)) {
			SRAL_GetEngineParameter(ENGINE_NONE, USE_CHARACTER_DESCRIPTIONS, &use_char_desc);
			printf("  New USE_CHARACTER_DESCRIPTIONS confirmed by get: %s\n", use_char_desc ? "Enabled" : "Disabled");
			CHECK(use_char_desc == new_use_char_desc, "USE_CHARACTER_DESCRIPTIONS set/get matches", "USE_CHARACTER_DESCRIPTIONS set/get mismatch");
			bool temp_spelling_state = true;
			SRAL_SetEngineParameter(ENGINE_NONE, ENABLE_SPELLING, &temp_spelling_state);
			SRAL_Speak("Testing character descriptions: A B C", true);
			sleep_ms(3000);
			SRAL_SetEngineParameter(ENGINE_NONE, USE_CHARACTER_DESCRIPTIONS, &original_use_char_desc);
			printf("  Restored original USE_CHARACTER_DESCRIPTIONS.\n");
		}
		else {
			printf("  Failed to set USE_CHARACTER_DESCRIPTIONS (might not be supported or settable).\n");
		}
	}
	else {
		printf("  Failed to get initial USE_CHARACTER_DESCRIPTIONS (might not be supported).\n");
	}


	TEST_SECTION("SRAL_Delay");
	prompt_user("Prepare for delay test. 'First message.' will speak, then a 3-second delay, then 'Second message after delay.'");
	SRAL_Speak("First message.", true);
	SRAL_Delay(3000);
	SRAL_Speak("Second message after delay.", true);
	prompt_user("Delay test finished. Press Enter to continue.");
	SRAL_StopSpeech(); // Good practice to stop, also clear delay queue.


	TEST_SECTION("Unregister Keyboard Hooks");
	SRAL_UnregisterKeyboardHooks();
	printf("SRAL_UnregisterKeyboardHooks called. Hooks should now be inactive (if they were active).\n");
	prompt_user("Try Ctrl/Shift with next speech to confirm hooks are off (if they were previously working).");
	SRAL_Speak("Testing speech output after attempting to unregister keyboard hooks.", true);
	sleep_ms(3000);


	TEST_SECTION("SRAL_Uninitialize");
	SRAL_Uninitialize();
	printf("SRAL_Uninitialize called.\n");
	CHECK(!SRAL_IsInitialized(), "SRAL_IsInitialized correctly returns false after uninit.", "SRAL_IsInitialized returned true after uninit!");

	printf("\nAttempting to speak after uninitialization (should fail or do nothing):\n");
	if (SRAL_Speak("This speech should not happen.", false)) {
		printf("[WARNING] SRAL_Speak appeared to succeed after uninitialization!\n");
	}
	else {
		printf("[INFO] SRAL_Speak correctly failed or did nothing after uninitialization.\n");
	}

	prompt_user("All tests complete. Press Enter to exit.");
	return 0;
}
