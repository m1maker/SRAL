package main

import (
	"SRAL"
	"bufio"
	"fmt"
	"math"
	"os"
	"time"
)

func promptUser(message string) {
	fmt.Printf("\n>>> %s (Press Enter to continue)...", message)
	reader := bufio.NewReader(os.Stdin)
	_, err := reader.ReadBytes('\n')
	if err != nil {
		if err.Error() == "EOF" {
			fmt.Println("EOF detected on stdin, continuing without prompt.")
		}
	}
}

func PrintEngineNames(engineBitmask SRAL.Engine, title string) {
	fmt.Printf("%s:\n", title)
	if engineBitmask == SRAL.NoneEngine {
		fmt.Printf("  (None)\n")
		return
	}
	found := false
	var engine_val SRAL.Engine
	for engine_val = SRAL.NVDAEngine; engine_val <= SRAL.AVSpeechEngine; engine_val <<= 1 {
		if engineBitmask&engine_val != 0 {
			name := SRAL.GetEngineName(engine_val)
			fmt.Printf("  - %s (%d)\n", name, int(engine_val))
			found = true
		}
	}
	if !found && engineBitmask != SRAL.NoneEngine {
		fmt.Printf("  (Unknown bitmask: %d)\n", int(engineBitmask))
	}
	fmt.Printf("\n")
}

func printSupportedFeatures(features SRAL.Feature) {
	fmt.Printf("Supported Features (%d):\n", int(features))
	if features == SRAL.NoneFeature {
		fmt.Printf("  (None)\n")
		return
	}
	if features.IsSupported(SRAL.SpeechFeature) {
		fmt.Printf("  - SUPPORTS_SPEECH\n")
	}
	if features.IsSupported(SRAL.BrailleFeature) {
		fmt.Printf("  - SUPPORTS_BRAILLE\n")
	}
	if features.IsSupported(SRAL.SpeechRateFeature) {
		fmt.Printf("  - SUPPORTS_SPEECH_RATE\n")
	}
	if features.IsSupported(SRAL.SpeechVolumeFeature) {
		fmt.Printf("  - SUPPORTS_SPEECH_VOLUME\n")
	}
	if features.IsSupported(SRAL.SelectVoiceFeature) {
		fmt.Printf("  - SUPPORTS_SELECT_VOICE\n")
	}
	if features.IsSupported(SRAL.PauseSpeechFeature) {
		fmt.Printf("  - SUPPORTS_PAUSE_SPEECH\n")
	}
	if features.IsSupported(SRAL.SSMLFeature) {
		fmt.Printf("  - SUPPORTS_SSML\n")
	}
	if features.IsSupported(SRAL.SpeakToMemoryFeature) {
		fmt.Printf("  - SUPPORTS_SPEAK_TO_MEMORY\n")
	}
	if features.IsSupported(SRAL.SpellingFeature) {
		fmt.Printf("  - SUPPORTS_SPELLING\n")
	}
	fmt.Printf("\n")
}

func TestSection(name string) {
	fmt.Printf("\n\n========================================\n")
	fmt.Printf("  Testing: %s\n", name)
	fmt.Printf("========================================\n")
}

func Check(condition bool, success_msg, fail_msg string) {
	if condition {
		fmt.Printf("[SUCCESS] %s\n", success_msg)
	} else {
		fmt.Printf("[FAILURE] %s\n", fail_msg)
	}
}

func CheckSRAL(condition bool, action_desc string) {
	if condition {
		fmt.Printf("[SUCCESS] %s\n", action_desc)
	} else {
		fmt.Printf("[FAILURE] %s\n", action_desc)
	}
}

func flagStatus(fl bool) string {
	if fl {
		return "Enabled"
	}
	return "Disabled"
}

func main() {
	fmt.Printf("SRAL Tester\n")
	fmt.Printf("-------------------------\n")
	TestSection("SRAL_IsInitialized (Before Initialization)")
	Check(!SRAL.IsInitialized(), "SRAL_IsInitialized correctly returns false before init.", "SRAL_IsInitialized returned true before init!")
	TestSection("SRAL_Initialize")
	engines_to_exclude := SRAL.NoneEngine
	engines_to_exclude = SRAL.UIAEngine
	fmt.Printf("Attempting to initialize SRAL, excluding engines: %d (%s)\n",
		engines_to_exclude, SRAL.GetEngineName(engines_to_exclude))

	if SRAL.Initialize(engines_to_exclude) {
		fmt.Printf("[SUCCESS] SRAL_Initialize successful.\n")
	} else {
		fmt.Printf("[FAILURE] SRAL_Initialize failed. Some tests may not run or behave as expected. Exiting.\n")
		return
	}
	Check(SRAL.IsInitialized(), "SRAL_IsInitialized correctly returns true after init.", "SRAL_IsInitialized returned false after init!")
	TestSection("Engine Information")
	available_engines := SRAL.GetAvailableEngines()
	PrintEngineNames(available_engines, "Available Engines on this Platform")

	active_engines := SRAL.GetActiveEngines()
	PrintEngineNames(active_engines, "Currently Active/Usable Engines")

	current_engine_id := SRAL.GetCurrentEngine()
	fmt.Printf("Current Default Engine: %s (%d)\n", SRAL.GetEngineName(current_engine_id), current_engine_id)

	fmt.Printf("\nNames of all SRAL_Engines enum members (as per SRAL_GetEngineName):\n")
	for e_val := SRAL.NVDAEngine; e_val <= SRAL.AVSpeechEngine; e_val <<= 1 {
		name := SRAL.GetEngineName(SRAL.Engine(e_val))
		fmt.Printf("  Engine ID %d: %s\n", e_val, name)
	}

	specific_engine_for_ex_tests := SRAL.NoneEngine
	if active_engines != SRAL.NoneEngine {
		for e_val := SRAL.NVDAEngine; e_val <= SRAL.AVSpeechEngine; e_val <<= 1 {
			e := SRAL.Engine(e_val)
			if ((active_engines & e) != 0) && e != current_engine_id {
				specific_engine_for_ex_tests = e
				break
			}
		}
		if specific_engine_for_ex_tests == SRAL.NoneEngine {
			for e_val := SRAL.NVDAEngine; e_val <= SRAL.AVSpeechEngine; e_val <<= 1 {
				e := SRAL.Engine(e_val)
				if (active_engines & e) != 0 {
					specific_engine_for_ex_tests = e
					break
				}
			}
		}
	}

	if specific_engine_for_ex_tests != SRAL.NoneEngine {
		fmt.Printf("\nWill use engine '%s' (%d) for specific engine (Ex) tests.\n",
			SRAL.GetEngineName(specific_engine_for_ex_tests), specific_engine_for_ex_tests)
	} else {
		fmt.Printf("\nNo specific engine distinct from default (or no active engines) for Ex tests. Ex tests might target default or fail if engine doesn't support action.\n")
	}

	TestSection("Keyboard Hooks")

	if SRAL.RegisterKeyboardHooks() {
		fmt.Printf("[SUCCESS] SRAL_RegisterKeyboardHooks registered.\n")
		promptUser("Keyboard hooks (Ctrl=Interrupt, Shift=Pause/Resume) are active. Test them with upcoming speech. This primarily affects non-screen-reader engines like SAPI or SpeechDispatcher.")
	} else {
		fmt.Printf("[INFO] SRAL_RegisterKeyboardHooks failed or did not register. This might be expected if no suitable engine is active or if permissions are lacking.\n")
	}

	TestSection("SRAL_GetEngineFeatures")
	fmt.Printf("Features for Current Default Engine (%s):\n", SRAL.GetEngineName(current_engine_id))
	current_engine_features := SRAL.GetEngineFeatures(SRAL.NoneEngine)
	printSupportedFeatures(current_engine_features)

	if specific_engine_for_ex_tests != SRAL.NoneEngine {
		fmt.Printf("Features for Specific Engine selected for Ex tests (%s):\n", SRAL.GetEngineName(specific_engine_for_ex_tests))
		specific_engine_features := SRAL.GetEngineFeatures(specific_engine_for_ex_tests)
		printSupportedFeatures(specific_engine_features)
	}

	if current_engine_features.IsSupported(SRAL.SpeechFeature) {
		TestSection("SRAL_Speak (Default Engine)")
		CheckSRAL(SRAL.Speak("Testing SRAL Speak, not interrupting previous speech.", false), "SRAL_Speak (no interrupt)")
		time.Sleep(2000 * time.Millisecond)
		CheckSRAL(SRAL.Speak("Testing SRAL Speak, interrupting previous speech.", true), "SRAL_Speak (interrupt)")
		time.Sleep(2000 * time.Millisecond)

		if specific_engine_for_ex_tests != SRAL.NoneEngine {
			TestSection("SRAL_SpeakEx (Specific Engine)")
			features_ex := SRAL.GetEngineFeatures(specific_engine_for_ex_tests)
			if features_ex.IsSupported(SRAL.SpeechFeature) {
				CheckSRAL(SRAL.SpeakEx(specific_engine_for_ex_tests, "Testing SRAL SpeakEx, not interrupting.", false), "SRAL_SpeakEx (no interrupt)")
				time.Sleep(2000 * time.Millisecond)
				CheckSRAL(SRAL.SpeakEx(specific_engine_for_ex_tests, "Testing SRAL SpeakEx, interrupting.", true), "SRAL_SpeakEx (interrupt)")
				time.Sleep(2000 * time.Millisecond)
			} else {
				fmt.Printf("Specific engine %s does not support speech for SpeakEx.\n", SRAL.GetEngineName(specific_engine_for_ex_tests))
			}
		}
	} else {
		fmt.Printf("Current default engine does not support speech. Skipping speech tests.\n")
	}

	if current_engine_features.IsSupported(SRAL.SSMLFeature) {
		TestSection("SRAL_SpeakSsml (Default Engine)")
		ssml_test := "<speak>This is <prosody pitch='150%'>SSML</prosody> text.</speak>"
		CheckSRAL(SRAL.SpeakSsml(ssml_test, true), "SRAL_SpeakSsml")
		time.Sleep(3000 * time.Millisecond)

		if specific_engine_for_ex_tests != SRAL.NoneEngine {
			TestSection("SRAL_SpeakSsmlEx (Specific Engine)")
			features_ex := SRAL.GetEngineFeatures(specific_engine_for_ex_tests)
			if features_ex.IsSupported(SRAL.SSMLFeature) {
				CheckSRAL(SRAL.SpeakSsmlEx(specific_engine_for_ex_tests, ssml_test, true), "SRAL_SpeakSsmlEx")
				time.Sleep(3000 * time.Millisecond)
			} else {
				fmt.Printf("Specific engine %s does not support SSML for SpeakSsmlEx.\n", SRAL.GetEngineName(specific_engine_for_ex_tests))
			}
		}
	} else {
		fmt.Printf("Current default engine does not support SSML. Skipping SSML tests.\n")
	}

	if current_engine_features.IsSupported(SRAL.SpeakToMemoryFeature) {
		TestSection("SRAL_SpeakToMemory (Default Engine)")
		pcm_data, err := SRAL.SpeakToMemory("Testing speak to memory audio synthesis.")
		if err == nil {
			fmt.Printf("[SUCCESS] SRAL_SpeakToMemory successful.\n")
			fmt.Printf("  Buffer Size: %d bytes\n", len(pcm_data.Buffer))
			fmt.Printf("  Channels: %d\n", pcm_data.Channels)
			fmt.Printf("  Sample Rate: %d Hz\n", pcm_data.SampleRate)
			fmt.Printf("  Bits Per Sample: %d\n", pcm_data.BitsPerSample)
			fmt.Printf("  No release required for PCM buffer.\n")
		} else {
			fmt.Printf("[FAILURE] SRAL_SpeakToMemory failed.\n")
		}

		if specific_engine_for_ex_tests != SRAL.NoneEngine {
			TestSection("SRAL_SpeakToMemoryEx (Specific Engine)")
			features_ex := SRAL.GetEngineFeatures(specific_engine_for_ex_tests)
			if features_ex.IsSupported(SRAL.SpeakToMemoryFeature) {
				pcm_data, err = SRAL.SpeakToMemoryEx(specific_engine_for_ex_tests, "Testing speak to memory ex.")
				if err == nil {
					fmt.Printf("[SUCCESS] SRAL_SpeakToMemoryEx successful for engine %s.\n", SRAL.GetEngineName(specific_engine_for_ex_tests))
					fmt.Printf("  Buffer Size: %d bytes\n", len(pcm_data.Buffer))
					fmt.Printf("  Channels: %d\n", pcm_data.Channels)
					fmt.Printf("  Sample Rate: %d Hz\n", pcm_data.SampleRate)
					fmt.Printf("  Bits Per Sample: %d\n", pcm_data.BitsPerSample)
					fmt.Printf("  No release required for PCM buffer.\n")
				} else {
					fmt.Printf("[FAILURE] SRAL_SpeakToMemoryEx failed for engine %s.\n", SRAL.GetEngineName(specific_engine_for_ex_tests))
				}
			} else {
				fmt.Printf("Specific engine %s does not support Speak To Memory for SpeakToMemoryEx.\n", SRAL.GetEngineName(specific_engine_for_ex_tests))
			}
		}
	} else {
		fmt.Printf("Current default engine does not support Speak To Memory. Skipping these tests.\n")
	}

	if current_engine_features.IsSupported(SRAL.BrailleFeature) {
		TestSection("SRAL_Braille (Default Engine)")
		promptUser("Prepare to check Braille display for 'Testing SRAL Braille output.'")
		CheckSRAL(SRAL.Braille("Testing SRAL Braille output."), "SRAL_Braille")

		if specific_engine_for_ex_tests != SRAL.NoneEngine {
			TestSection("SRAL_BrailleEx (Specific Engine)")
			features_ex := SRAL.GetEngineFeatures(specific_engine_for_ex_tests)
			if features_ex.IsSupported(SRAL.BrailleFeature) {
				promptUser("Prepare to check Braille display for 'Testing SRAL Braille Ex output.'")
				CheckSRAL(SRAL.BrailleEx(specific_engine_for_ex_tests, "Testing SRAL Braille Ex output."), "SRAL_BrailleEx")
			} else {
				fmt.Printf("Specific engine %s does not support Braille for BrailleEx.\n", SRAL.GetEngineName(specific_engine_for_ex_tests))
			}
		}
	} else {
		fmt.Printf("Current default engine does not support Braille. Skipping Braille tests.\n")
	}

	TestSection("SRAL_Output (Default Engine)")
	promptUser("Prepare for SRAL_Output (Speech and/or Braille) for 'Testing SRAL Output, not interrupting.'")
	CheckSRAL(SRAL.Output("Testing SRAL Output, not interrupting.", false), "SRAL_Output (no interrupt)")
	time.Sleep(2000 * time.Millisecond)
	promptUser("Prepare for SRAL_Output (Speech and/or Braille) for 'Testing SRAL Output, interrupting.'")
	CheckSRAL(SRAL.Output("Testing SRAL Output, interrupting now.", true), "SRAL_Output (interrupt)")
	time.Sleep(2000 * time.Millisecond)

	if specific_engine_for_ex_tests != SRAL.NoneEngine {
		TestSection("SRAL_OutputEx (Specific Engine)")
		promptUser("Prepare for SRAL_OutputEx with specific engine for 'Testing SRAL OutputEx, not interrupting.'")
		CheckSRAL(SRAL.OutputEx(specific_engine_for_ex_tests, "Testing SRAL OutputEx, not interrupting.", false), "SRAL_OutputEx (no interrupt)")
		time.Sleep(2000 * time.Millisecond)
		promptUser("Prepare for SRAL_OutputEx with specific engine for 'Testing SRAL OutputEx, interrupting.'")
		CheckSRAL(SRAL.OutputEx(specific_engine_for_ex_tests, "Testing SRAL OutputEx, interrupting now.", true), "SRAL_OutputEx (interrupt)")
		time.Sleep(2000 * time.Millisecond)
	}

	if current_engine_features.IsSupported(SRAL.SpeechFeature) {
		TestSection("Speech Control (Default Engine)")
		long_speech := "This is a moderately long sentence designed to test the pause, resume, and stop functionality of the SRAL library effectively."
		fmt.Printf("Speaking long sentence with default engine: \"%s\"\n", long_speech)
		SRAL.Speak(long_speech, true)
		promptUser("Speech started. Press Enter to attempt PAUSE (if supported).")
		status := "False"
		if SRAL.IsSpeaking() {
			status = "true"
		}
		fmt.Printf("IsSpeaking status: %s", status)

		if current_engine_features.IsSupported(SRAL.PauseSpeechFeature) {
			CheckSRAL(SRAL.PauseSpeech(), "SRAL_PauseSpeech")
			promptUser("Speech Paused (hopefully). Press Enter to attempt RESUME.")
			CheckSRAL(SRAL.ResumeSpeech(), "SRAL_ResumeSpeech")
			promptUser("Speech Resumed (hopefully). Press Enter to STOP.")
		} else {
			fmt.Printf("Pause/Resume not supported by current default engine according to features. Will attempt stop directly.\n")
			promptUser("Speech should be ongoing. Press Enter to STOP.")
		}
		CheckSRAL(SRAL.StopSpeech(), "SRAL_StopSpeech")
		fmt.Printf("Speech should be stopped now.\n")
		time.Sleep(500 * time.Millisecond)

		if specific_engine_for_ex_tests != SRAL.NoneEngine {
			TestSection("Speech Control Ex (Specific Engine)")
			features_ex := SRAL.GetEngineFeatures(specific_engine_for_ex_tests)
			if features_ex.IsSupported(SRAL.SpeechFeature) {
				fmt.Printf("Speaking long sentence with engine %s: \"%s\"\n", SRAL.GetEngineName(specific_engine_for_ex_tests), long_speech)
				SRAL.SpeakEx(specific_engine_for_ex_tests, long_speech, true)
				promptUser("Speech started (Ex). Press Enter to PAUSE (Ex) (if supported).")
				status := "False"
				if SRAL.IsSpeakingEx(specific_engine_for_ex_tests) {
					status = "true"
				}
				fmt.Printf("IsSpeaking status: %s", status)

				if features_ex.IsSupported(SRAL.PauseSpeechFeature) {
					CheckSRAL(SRAL.PauseSpeechEx(specific_engine_for_ex_tests), "SRAL_PauseSpeechEx")
					promptUser("Speech Paused (Ex). Press Enter to RESUME (Ex).")
					CheckSRAL(SRAL.ResumeSpeechEx(specific_engine_for_ex_tests), "SRAL_ResumeSpeechEx")
					promptUser("Speech Resumed (Ex). Press Enter to STOP (Ex).")
				} else {
					fmt.Printf("Pause/Resume not supported by specific engine %s. Will attempt stop directly.\n", SRAL.GetEngineName(specific_engine_for_ex_tests))
					promptUser("Speech should be ongoing (Ex). Press Enter to STOP (Ex).")
				}
				CheckSRAL(SRAL.StopSpeechEx(specific_engine_for_ex_tests), "SRAL_StopSpeechEx")
				fmt.Printf("Speech should be stopped (Ex).\n")
				time.Sleep(500 * time.Millisecond)
			} else {
				fmt.Printf("Specific engine %s does not support speech. Skipping Speech Control Ex tests.\n", SRAL.GetEngineName(specific_engine_for_ex_tests))
			}
		}
	}

	TestSection("SRAL Engine Parameters (Default Engine)")

	if current_engine_features.IsSupported(SRAL.SpeechRateFeature) {
		fmt.Printf("\nTesting SPEECH_RATE (Default Engine):\n")
		var original_rate int
		if SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.SpeechRateParam, &original_rate) {
			fmt.Printf("  Original rate: %d\n", original_rate)
			new_rate := original_rate + 10
			if original_rate > 90 {
				new_rate = original_rate - 10
			}
			if new_rate < 0 {
				new_rate = 0
			}
			if new_rate > 100 {
				new_rate = 100
			}

			fmt.Printf("  Attempting to set rate to: %d\n", new_rate)
			if SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.SpeechRateParam, new_rate) {
				var fetched_rate int
				SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.SpeechRateParam, &fetched_rate)
				fmt.Printf("  New rate confirmed by get: %d\n", fetched_rate)
				Check(math.Abs(float64(fetched_rate-new_rate)) <= 5, "Rate set and get matches (or is close)", "Rate set/get mismatch or significant difference")
				SRAL.Speak("Testing new speech rate setting.", true)
				time.Sleep(2500 * time.Millisecond)
				SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.SpeechRateParam, original_rate)
				fmt.Printf("  Restored original rate to: %d\n", original_rate)
			} else {
				fmt.Printf("  Failed to set SPEECH_RATE.\n")
			}
		} else {
			fmt.Printf("  Failed to get initial SPEECH_RATE.\n")
		}
	} else {
		fmt.Printf("\nSPEECH_RATE not supported by current default engine.\n")
	}

	if current_engine_features.IsSupported(SRAL.SpeechVolumeFeature) {
		fmt.Printf("\nTesting SPEECH_VOLUME (Default Engine):\n")
		var original_volume int
		if SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.SpeechVolumeParam, &original_volume) {
			fmt.Printf("  Original volume: %d\n", original_volume)
			new_volume := original_volume + 10
			if original_volume > 90 {
				new_volume = original_volume - 10
			}
			if new_volume < 0 {
				new_volume = 0
			}
			if new_volume > 100 {
				new_volume = 100
			}

			fmt.Printf("  Attempting to set volume to: %d\n", new_volume)
			if SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.SpeechVolumeParam, new_volume) {
				var fetched_volume int
				SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.SpeechVolumeParam, &fetched_volume)
				fmt.Printf("  New volume confirmed by get: %d\n", fetched_volume)
				Check(math.Abs(float64(fetched_volume-new_volume)) <= 5, "Volume set and get matches (or is close)", "Volume set/get mismatch or significant difference")
				SRAL.Speak("Testing new speech volume setting.", true)
				time.Sleep(2500 * time.Millisecond)
				SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.SpeechVolumeParam, original_volume)
				fmt.Printf("  Restored original volume to: %d\n", original_volume)
			} else {
				fmt.Printf("  Failed to set SPEECH_VOLUME.\n")
			}
		} else {
			fmt.Printf("  Failed to get initial SPEECH_VOLUME.\n")
		}
	} else {
		fmt.Printf("\nSPEECH_VOLUME not supported by current default engine.\n")
	}

	if current_engine_features.IsSupported(SRAL.SelectVoiceFeature) {
		fmt.Printf("\nTesting VOICE parameters (Default Engine):\n")
		voice_count := 0
		if SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.VoiceCountParam, &voice_count) {
			fmt.Printf("  Voice count: %d\n", voice_count)
			if voice_count > 0 {
				voice_infos := []SRAL.VoiceInfo{}
				if SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.VoicePropertiesParam, &voice_infos) {
					fmt.Printf("  Available voices:\n")
					for i := 0; i < voice_count; i++ {
						fmt.Printf("    %d: %s\n", i, voice_infos[i].Name)
						fmt.Printf("    %d: %s\n", i, voice_infos[i].Language)
						fmt.Printf("    %d: %s\n", i, voice_infos[i].Gender)
						fmt.Printf("    %d: %s\n", i, voice_infos[i].Vendor)
					}

					current_voice_index := -1
					original_voice_index := -1
					SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.VoiceIndexParam, &original_voice_index)
					if original_voice_index < 0 || original_voice_index > voice_count-1 {
						fmt.Printf("  Current voice index: %d (Unknown/default)\n", original_voice_index)
					} else {
						fmt.Printf("  Current voice index: %d (%s)\n", original_voice_index, voice_infos[original_voice_index].Name)
					}

					if voice_count > 1 {
						new_voice_index := (original_voice_index + 1) % voice_count
						fmt.Printf("  Attempting to set voice to index: %d (%s)\n", new_voice_index, voice_infos[new_voice_index].Name)
						if SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.VoiceIndexParam, new_voice_index) {
							SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.VoiceIndexParam, &current_voice_index)
							fmt.Printf("  New voice index confirmed by get: %d\n", current_voice_index)
							Check(current_voice_index == new_voice_index, "Voice index set and get matches", "Voice index set/get mismatch")
							SRAL.Speak("Testing newly selected voice.", true)
							time.Sleep(3000 * time.Millisecond)
							if original_voice_index != -1 {
								SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.VoiceIndexParam, original_voice_index)
								fmt.Printf("  Restored original voice index to: %d\n", original_voice_index)
							}
						} else {
							fmt.Printf("  Failed to set VOICE_INDEX.\n")
						}
					}
				}
				// free no required
			}
		} else {
			fmt.Printf("  Failed to get VOICE_COUNT.\n")
		}
	} else {
		fmt.Printf("\nVOICE selection not supported by current default engine.\n")
	}

	fmt.Printf("\nTesting ENABLE_SPELLING (Default Engine):\n")
	var spelling_enabled, original_spelling_state bool
	if SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.EnableSpellingParam, &original_spelling_state) {
		fmt.Printf("  Initial spelling state: %s\n", flagStatus(original_spelling_state))
		new_spelling_state := !original_spelling_state
		fmt.Printf("  Attempting to set spelling to: %s\n", flagStatus(new_spelling_state))
		if SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.EnableSpellingParam, new_spelling_state) {
			SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.EnableSpellingParam, &spelling_enabled)
			fmt.Printf("  New spelling state confirmed by get: %s\n", flagStatus(spelling_enabled))
			Check(spelling_enabled == new_spelling_state, "Spelling state set/get matches", "Spelling state set/get mismatch")
			if new_spelling_state {
				SRAL.Speak("Spelling test: H E L L O", true)
			} else {
				SRAL.Speak("Spelling test: Hello", true)
			}
			time.Sleep(3000 * time.Millisecond)
			SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.EnableSpellingParam, original_spelling_state)
			fmt.Printf("  Restored original spelling state.\n")
		} else {
			fmt.Printf("  Failed to set ENABLE_SPELLING (This might be normal if not truly settable by this engine, or if feature isn't fully supported).\n")
		}
	} else {
		fmt.Printf("  Failed to get initial ENABLE_SPELLING (This might be normal if not supported by current engine).\n")
	}

	if current_engine_id == SRAL.NVDAEngine || (specific_engine_for_ex_tests != SRAL.NoneEngine && specific_engine_for_ex_tests == SRAL.NVDAEngine) {
		fmt.Printf("\nTesting NVDA_IS_CONTROL_EX (for NVDA):\n")
		var nvda_is_control_ex_val bool
		if SRAL.GetEngineParameter(SRAL.NVDAEngine, SRAL.NVDAIsControlExParam, &nvda_is_control_ex_val) {
			status := "false (standard control)"
			if nvda_is_control_ex_val {
				status = "true (extended control available)"
			}
			fmt.Printf("  NVDA_IS_CONTROL_EX value from NVDA engine: %s\n", status)
		} else {
			fmt.Printf("  Failed to get NVDA_IS_CONTROL_EX (NVDA might not be the one responding or parameter not available).\n")
		}
	}

	if current_engine_id == SRAL.SAPIEngine || (specific_engine_for_ex_tests != SRAL.NoneEngine && specific_engine_for_ex_tests == SRAL.SAPIEngine) {
		fmt.Printf("\nTesting SAPI_TRIM_THRESHOLD (for SAPI):\n")
		var trim_threshold, original_trim_threshold int
		if SRAL.GetEngineParameter(SRAL.SAPIEngine, SRAL.SAPITrimThresholdParam, &original_trim_threshold) {
			fmt.Printf("  Initial SAPI_TRIM_THRESHOLD: %d\n", original_trim_threshold)
			new_threshold := 20
			if original_trim_threshold == 20 {
				new_threshold = 60
			}
			fmt.Printf("  Attempting to set SAPI_TRIM_THRESHOLD to: %d\n", new_threshold)
			if SRAL.SetEngineParameter(SRAL.SAPIEngine, SRAL.SAPITrimThresholdParam, new_threshold) {
				SRAL.GetEngineParameter(SRAL.SAPIEngine, SRAL.SAPITrimThresholdParam, &trim_threshold)
				fmt.Printf("  New SAPI_TRIM_THRESHOLD confirmed by get: %d\n", trim_threshold)
				Check(trim_threshold == new_threshold, "SAPI_TRIM_THRESHOLD set/get matches", "SAPI_TRIM_THRESHOLD set/get mismatch")
				SRAL.SpeakEx(SRAL.SAPIEngine, "Testing SAPI trim threshold. ... Some silence here.", true)
				time.Sleep(3000 * time.Millisecond)
				SRAL.SetEngineParameter(SRAL.SAPIEngine, SRAL.SAPITrimThresholdParam, original_trim_threshold)
				fmt.Printf("  Restored SAPI_TRIM_THRESHOLD.\n")
			} else {
				fmt.Printf("  Failed to set SAPI_TRIM_THRESHOLD.\n")
			}
		} else {
			fmt.Printf("  Failed to get initial SAPI_TRIM_THRESHOLD (SAPI might not be responding or param not available).\n")
		}
	}

	fmt.Printf("\nTesting SYMBOL_LEVEL (Default Engine):\n")
	var symbol_level, original_symbol_level int
	if SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.SymbolLevelParam, &original_symbol_level) {
		fmt.Printf("  Original symbol_level: %d\n", original_symbol_level)
		new_symbol_level := (original_symbol_level + 1) % 4
		fmt.Printf("  Attempting to set symbol_level to: %d\n", new_symbol_level)
		if SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.SymbolLevelParam, new_symbol_level) {
			SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.SymbolLevelParam, &symbol_level)
			fmt.Printf("  New symbol_level confirmed by get: %d\n", symbol_level)
			Check(symbol_level == new_symbol_level, "Symbol level set/get matches", "Symbol level set/get mismatch")
			SRAL.Speak("Testing symbol level with punctuation! At symbol @ hash # dollar $ percent % caret ^ ampersand & asterisk *.", true)
			time.Sleep(4000 * time.Millisecond)
			SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.SymbolLevelParam, original_symbol_level)
			fmt.Printf("  Restored original symbol_level: %d\n", original_symbol_level)
		} else {
			fmt.Printf("  Failed to set SYMBOL_LEVEL (might not be supported or settable by this engine).\n")
		}
	} else {
		fmt.Printf("  Failed to get initial SYMBOL_LEVEL (might not be supported by current engine).\n")
	}

	fmt.Printf("\nTesting USE_CHARACTER_DESCRIPTIONS (Default Engine):\n")
	var use_char_desc, original_use_char_desc bool
	if SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.UseCharacterDescriptionsParam, &original_use_char_desc) {
		fmt.Printf("  Initial USE_CHARACTER_DESCRIPTIONS: %s\n", flagStatus(original_use_char_desc))
		new_use_char_desc := !original_use_char_desc
		fmt.Printf("  Attempting to set USE_CHARACTER_DESCRIPTIONS to: %s\n", flagStatus(new_use_char_desc))
		if SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.UseCharacterDescriptionsParam, new_use_char_desc) {
			SRAL.GetEngineParameter(SRAL.NoneEngine, SRAL.UseCharacterDescriptionsParam, &use_char_desc)
			fmt.Printf("  New USE_CHARACTER_DESCRIPTIONS confirmed by get: %s\n", flagStatus(use_char_desc))
			Check(use_char_desc == new_use_char_desc, "USE_CHARACTER_DESCRIPTIONS set/get matches", "USE_CHARACTER_DESCRIPTIONS set/get mismatch")
			temp_spelling_state := true
			SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.EnableSpellingParam, temp_spelling_state)
			SRAL.Speak("Testing character descriptions: A B C", true)
			time.Sleep(3000 * time.Millisecond)
			SRAL.SetEngineParameter(SRAL.NoneEngine, SRAL.UseCharacterDescriptionsParam, original_use_char_desc)
			fmt.Printf("  Restored original USE_CHARACTER_DESCRIPTIONS.\n")
		} else {
			fmt.Printf("  Failed to set USE_CHARACTER_DESCRIPTIONS (might not be supported or settable).\n")
		}
	} else {
		fmt.Printf("  Failed to get initial USE_CHARACTER_DESCRIPTIONS (might not be supported).\n")
	}

	TestSection("SRAL_Delay")
	promptUser("Prepare for delay test. 'First message.' will speak, then a 3-second delay, then 'Second message after delay.'")
	SRAL.Speak("First message.", true)
	SRAL.Delay(3000 * time.Millisecond)
	SRAL.Speak("Second message after delay.", true)
	promptUser("Delay test finished. Press Enter to continue.")
	SRAL.StopSpeech()

	TestSection("SRAL_Set/GetEnginesExclude")

	original_engines_to_exclude := engines_to_exclude
	engines_to_exclude = SRAL.NVDAEngine | SRAL.SAPIEngine
	fmt.Printf("Original excluding engines: %d (%s)\n",
		original_engines_to_exclude, SRAL.GetEngineName(original_engines_to_exclude))

	fmt.Printf("Attempting to exclude engines: %d (%s)\n",
		engines_to_exclude, SRAL.GetEngineName(engines_to_exclude))

	Check(SRAL.SetEnginesExclude(SRAL.NVDAEngine|SRAL.SAPIEngine), "Excludes was successfully set.", "Failed to set engines excludes")
	new_engines_to_exclude := SRAL.GetEnginesExclude()
	fmt.Printf("  New excludes confirmed by get: %d (%s)\n",
		engines_to_exclude, SRAL.GetEngineName(new_engines_to_exclude))

	Check(engines_to_exclude == new_engines_to_exclude, "Engines exclude set/get matches", "Engines exclude set/get mismatch")

	TestSection("Unregister Keyboard Hooks")
	SRAL.UnregisterKeyboardHooks()
	fmt.Printf("SRAL_UnregisterKeyboardHooks called. Hooks should now be inactive (if they were active).\n")
	promptUser("Try Ctrl/Shift with next speech to confirm hooks are off (if they were previously working).")
	SRAL.Speak("Testing speech output after attempting to unregister keyboard hooks.", true)
	time.Sleep(3000 * time.Millisecond)

	TestSection("SRAL_Uninitialize")
	SRAL.Uninitialize()
	fmt.Printf("SRAL_Uninitialize called.\n")
	Check(!SRAL.IsInitialized(), "SRAL_IsInitialized correctly returns false after uninit.", "SRAL_IsInitialized returned true after uninit!")

	fmt.Printf("\nAttempting to speak after uninitialization (should fail or do nothing):\n")
	if SRAL.Speak("This speech should not happen.", false) {
		fmt.Printf("[WARNING] SRAL_Speak appeared to succeed after uninitialization!\n")
	} else {
		fmt.Printf("[INFO] SRAL_Speak correctly failed or did nothing after uninitialization.\n")
	}

	promptUser("All tests complete. Press Enter to exit.")
}
