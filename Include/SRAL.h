/*
   SRAL (Screen Reader Abstraction Library)
   Copyright (c) 2024-2025 [m1maker]


   @file SRAL.h
   @brief This header file defines the Screen Reader Abstraction Library (SRAL).

   SRAL provides a unified interface for interacting with various speech engines.
   It abstracts the differences between multiple speech engines, allowing developers to
   implement accessibility features in their applications without needing to handle the
   specifics of each engine.

   This library supports multiple speech engines and offers a variety of features,
   enabling applications to provide auditory feedback, braille output, and control
   over speech parameters such as rate and volume.

*/
#ifndef SRAL_H_
#define SRAL_H_
#pragma once
#ifdef _WIN32
#if defined SRAL_EXPORT
#define SRAL_API __declspec(dllexport)
#elif defined (SRAL_STATIC)
#define SRAL_API
#else
#define SRAL_API __declspec(dllimport)
#endif
#else
#define SRAL_API
#endif
#ifdef __cplusplus
extern "C" {
#include <stdbool.h>
#endif
#include <stdint.h>
#include <stdlib.h>

	/**
 * @enum SRAL_Engines
 * @brief Enumeration of available speech engines.
 *
 * This enumeration defines the identifiers for different speech engines
 * supported by the SRAL library.
 */

	enum SRAL_Engines {
		SRAL_ENGINE_NONE = 0,
		SRAL_ENGINE_NVDA = 1 << 1,
		SRAL_ENGINE_SAPI = 1 << 2,
		SRAL_ENGINE_JAWS = 1 << 3,
		SRAL_ENGINE_SPEECH_DISPATCHER = 1 << 4,
		SRAL_ENGINE_UIA = 1 << 5,
		SRAL_ENGINE_AC_ANNOUNCER = 1 << 5,
		SRAL_ENGINE_AV_SPEECH = 1 << 6,
		SRAL_ENGINE_NARRATOR = 1 << 7
	};

	/**
 * @enum SRAL_SupportedFeatures
 * @brief Enumeration of supported features in the engines.
 *
 * This enumeration defines the features supported by various speech engines.
 */

	enum SRAL_SupportedFeatures {
		SRAL_SUPPORTS_SPEECH = 1 << 1,
		SRAL_SUPPORTS_BRAILLE = 1 << 2,
		SRAL_SUPPORTS_SPEECH_RATE = 1 << 3,
		SRAL_SUPPORTS_SPEECH_VOLUME = 1 << 4,
		SRAL_SUPPORTS_SELECT_VOICE = 1 << 5,
		SRAL_SUPPORTS_PAUSE_SPEECH = 1 << 6,
		SRAL_SUPPORTS_SSML = 1 << 7,
		SRAL_SUPPORTS_SPEAK_TO_MEMORY = 1 << 8,
		SRAL_SUPPORTS_SPELLING = 1 << 9
	};

	/**
* @enum SRAL_EngineParams
* @brief Enumeration of engine parameters.
*/

	enum SRAL_EngineParams {
		SRAL_PARAM_SPEECH_RATE,
		SRAL_PARAM_SPEECH_VOLUME,
		SRAL_PARAM_VOICE_INDEX,
		SRAL_PARAM_VOICE_LIST,
		SRAL_PARAM_VOICE_COUNT,
		SRAL_PARAM_SYMBOL_LEVEL,
		SRAL_PARAM_SAPI_TRIM_THRESHOLD,
		SRAL_PARAM_ENABLE_SPELLING,
		SRAL_PARAM_USE_CHARACTER_DESCRIPTIONS,
		SRAL_PARAM_NVDA_IS_CONTROL_EX
	};



	/**
	* @brief Speak the given text.
	* @param text A pointer to the text string to be spoken.
	* @param interrupt A flag indicating whether to interrupt the current speech.
	* @return true if speaking was successful, false otherwise.
	*/

	SRAL_API bool SRAL_Speak(const char* text, bool interrupt);


	/**
* @brief Speak the given text into memory.
* @param text A pointer to the text string to be spoken.
* @param buffer_size A pointer to uint64_t to write PCM buffer size.
* @param channels A pointer to int to write PCM channel count.
* @param sample_rate A pointer to int to write PCM sample rate in HZ.
* @param bits_per_sample A pointer to int to write PCM bit size (floating point or signed integer).


* @return a pointer to the PCM buffer if speaking was successful, false otherwise.
*/

	SRAL_API void* SRAL_SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample);

	/**
* @brief Speak the given text using SSML tags.
* @param SSML A pointer to the valid SSML string to be spoken.
* @param interrupt A flag indicating whether to interrupt the current speech.
* @return true if speaking was successful, false otherwise.
*/

	SRAL_API bool SRAL_SpeakSsml(const char* ssml, bool interrupt);




	/**
 * @brief Output text to a Braille display.
 * @param text A pointer to the text string to be output in Braille.
 * @return true if Braille output was successful, false otherwise.
 */

	SRAL_API bool SRAL_Braille(const char* text);

	/**
 * @brief Output text using all currently supported speech engine methods.
 * @param text A pointer to the text string to be output.
 * @param interrupt A flag indicating whether to interrupt speech.
 * @return true if output was successful, false otherwise.
 */

	SRAL_API bool SRAL_Output(const char* text, bool interrupt);


	/**
 * @brief Stop speech if it is active.
 * @return true if speech was stopped successfully, false otherwise.
 */


	SRAL_API bool SRAL_StopSpeech(void);

	/**
* @brief Pause speech if it is active and the current speech engine supports this.
* @return true if speech was paused successfully, false otherwise.
*/


	SRAL_API bool SRAL_PauseSpeech(void);


	/**
* @brief Resume speech if it was active and the current speech engine supports this.
* @return true if speech was resumed successfully, false otherwise.
*/


	SRAL_API bool SRAL_ResumeSpeech(void);




	/**
 * @brief Get the current speech engine in use.
 * @return The identifier of the current speech engine defined by the SRAL_Engines enumeration.
 */

	SRAL_API int SRAL_GetCurrentEngine(void);


	/**
 * @brief Get features supported by the specified engine.
 * @param engine The identifier of the engine to query. Defaults to 0 (current engine).
 * @return An integer representing the features supported by the engine defined by the SRAL_SupportedFeatures enumeration.
 */


	SRAL_API int SRAL_GetEngineFeatures(int engine);

	/**
* @brief Set the parameter for the specified speech engine.
* @param engine The engine to set the param for.
* @param param The desired parameter.
* @param value A pointer to desired value.

* @return true if the parameter was set successfully, false otherwise.
*/

	SRAL_API bool SRAL_SetEngineParameter(int engine, int param, const void* value);




	/**
* @brief Get the parameter for the specified speech engine.
* @param engine The engine to get the param for.
* @param value An out pointer to write value.

* @return true if the parameter was retrieved successfully, false otherwise.
*/

	SRAL_API bool SRAL_GetEngineParameter(int engine, int param, void* value);



	/**
 * @brief Initialize the library and optionally exclude certain engines.
 * @param engines_exclude A bitmask specifying engines to exclude from initialization. Defaults to 0 (include all).
 * @return true if initialization was successful, false otherwise.
 */


	SRAL_API bool SRAL_Initialize(int engines_exclude);

	/**
 * @brief Uninitialize the library, freeing resources.
 */

	SRAL_API void SRAL_Uninitialize(void);



	/**
 * Extended functions to perform operations with specific speech engines.
 */

 /**
  * @brief Speak the given text with the specified engine.
  * @param engine The engine to use for speaking.
  * @param text A pointer to the text string to be spoken.
  * @param interrupt A flag indicating whether to interrupt the current speech.
  * @return true if speaking was successful, false otherwise.
  */

	SRAL_API bool SRAL_SpeakEx(int engine, const char* text, bool interrupt);

	/**
* @brief Speak the given text into memory with the specified engine.
* @param engine The engine to use for speaking.
* @param text A pointer to the text string to be spoken.
* @param buffer_size A pointer to uint64_t to write PCM buffer size.
* @param channels A pointer to int to write PCM channel count.
* @param sample_rate A pointer to int to write PCM sample rate in HZ.
* @param bits_per_sample A pointer to int to write PCM bit size (floating point or signed integer).

* @return a pointer to the PCM buffer if speaking was successful, false otherwise.
*/

	SRAL_API void* SRAL_SpeakToMemoryEx(int engine, const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample);



	/**
 * @brief Speak the given text with the specified engine and using SSML tags.
 * @param engine The engine to use for speaking.
 * @param ssml A pointer to the valid SSML string to be spoken.
 * @param interrupt A flag indicating whether to interrupt the current speech.
 * @return true if speaking was successful, false otherwise.
 */

	SRAL_API bool SRAL_SpeakSsmlEx(int engine, const char* ssml, bool interrupt);




	/**
 * @brief Output text to a Braille display using the specified engine.
 * @param engine The engine to use for Braille display output.
 * @param text A pointer to the text string to be output in Braille display.
 * @return true if Braille output was successful, false otherwise.
 */


	SRAL_API bool SRAL_BrailleEx(int engine, const char* text);

	/**
 * @brief Output text using the specified engine.
 * @param engine The engine to use for output.
 * @param text A pointer to the text string to be output.
 * @param interrupt A flag indicating whether to interrupt the current speech.
 * @return true if output was successful, false otherwise.
 */

	SRAL_API bool SRAL_OutputEx(int engine, const char* text, bool interrupt);



	/**
 * @brief Stop speech for the specified engine.
 * @param engine The engine to stop speech for.
 * @return true if speech was stopped successfully, false otherwise.
 */


	SRAL_API bool SRAL_StopSpeechEx(int engine);


	/**
* @brief Pause speech for the specified engine.
* @param engine The engine to pause speech for.
* @return true if speech was paused successfully, false otherwise.
*/


	SRAL_API bool SRAL_PauseSpeechEx(int engine);


	/**
* @brief Resume speech for the specified engine.
* @param engine The engine to resume speech for.
* @return true if speech was resumed successfully, false otherwise.
*/



	SRAL_API bool SRAL_ResumeSpeechEx(int engine);



	/**
 * @brief Check if the library has been initialized.
 * @return true if the library is initialized, false otherwise.
 */

	SRAL_API bool SRAL_IsInitialized(void);

	/**

		// *@brief Delayes the next speech or output operation by a given time.
			* @param time A value in milliseconds.


			*/


	SRAL_API void SRAL_Delay(int time);




	/**
		*@brief Install speech interruption and pause keyboard hooks for speech engines other than screen readers, such as Microsoft SAPI 5 or SpeechDispatcher.
		* These hooks work globally in any window.
		* Ctrl - Interrupt, Shift - Pause.
			* @return true if the hooks are successfully installed, false otherwise.
			*/

	SRAL_API bool SRAL_RegisterKeyboardHooks(void);

	/**
		*@brief Uninstall speech interruption and pause keyboard hooks.
*/


	SRAL_API void SRAL_UnregisterKeyboardHooks(void);




	/**
* @brief Get all available engines for the current platform.
* @return Bitmask with available engines.
*/


	SRAL_API int SRAL_GetAvailableEngines(void);



	/**
* @brief Get all active engines that can be used.
* @return Bitmask with active engines.
*/


	SRAL_API int SRAL_GetActiveEngines(void);



	/**
 * @brief Get name of the specified engine.
 * @param engine The identifier of the engine to query.
 * @return a pointer to the name.
 */


	SRAL_API const char* SRAL_GetEngineName(int engine);




#ifdef __cplusplus
}// extern "C"
#endif

#endif // SRAL_H_