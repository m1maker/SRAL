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
		ENGINE_NONE = 0,
		ENGINE_NVDA = 2,
		ENGINE_SAPI = 4,
		ENGINE_JAWS = 8,
		ENGINE_SPEECH_DISPATCHER = 16,
		ENGINE_UIA = 32,
		ENGINE_AV_SPEECH = 64
	};

	/**
 * @enum SRAL_SupportedFeatures
 * @brief Enumeration of supported features in the engines.
 *
 * This enumeration defines the features supported by various speech engines.
 */

	enum SRAL_SupportedFeatures {
		SUPPORTS_SPEECH = 128,
		SUPPORTS_BRAILLE = 256,
		SUPPORTS_SPEECH_RATE = 512,
		SUPPORTS_SPEECH_VOLUME = 1024,
		SUPPORTS_SELECT_VOICE = 2048

	};

	/**
 * @brief Speak the given text.
 * @param text A pointer to the text string to be spoken.
 * @param interrupt A flag indicating whether to interrupt the current speech.
 * @return true if speaking was successful, false otherwise.
 */


	SRAL_API bool SRAL_Speak(const char* text, bool interrupt);



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
 * @brief Set the speech volume level, if current speech engine supports this.
 * @param value The desired volume level.
 * @return true if the volume was set successfully, false otherwise.
 */


	SRAL_API bool SRAL_SetVolume(uint64_t value);

	/**
 * @brief Get the current speech volume level of the current speech engine.
 * @return The current volume level.
 */

	SRAL_API uint64_t SRAL_GetVolume(void);


	/**
 * @brief Set the speech rate, if current engine supports this.
 * @param value The desired speech rate.
 * @return true if the speech rate was set successfully, false otherwise.
 */


	SRAL_API bool SRAL_SetRate(uint64_t value);

	/**
 * @brief Get the current speech rate of the current speech engine.
 * @return The current speech rate.
 */

	SRAL_API uint64_t SRAL_GetRate(void);

	/**
 * @brief Get the count of available voices supported by the current speech engine.
 * @return The number of available voices.
 */

	SRAL_API uint64_t SRAL_GetVoiceCount(void);

	/**
 * @brief Get the name of a voice by its index, if the current speech engine supports this.
 * @param The index of a voice to get.
 * @return A pointer to the name of the voice.
 */

	SRAL_API const char* SRAL_GetVoiceName(uint64_t index);

	/**
 * @brief Set the currently selected voice by index, if the current speech engine supports this.
 * @param index The index of a voice to set.
 * @return true if the voice was set successfully, false otherwise.
 */

	SRAL_API bool SRAL_SetVoice(uint64_t index);





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
 * @brief Set the volume level for the specified speech engine.
 * @param engine The engine to set the volume for.
 * @param value The desired volume level.
 * @return true if the volume was set successfully, false otherwise.
 */

	SRAL_API bool SRAL_SetVolumeEx(int engine, uint64_t value);


	/**
 * @brief Get the current volume level for the specified engine.
 * @param engine The engine to query.
 * @return The current volume level for the engine.
 */


	SRAL_API uint64_t SRAL_GetVolumeEx(int engine);

	/**
 * @brief Set the speech rate for the specified engine.
 * @param engine The engine to set the rate for.
 * @param value The desired speech rate.
 * @return true if the speech rate was set successfully, false otherwise.
 */

	SRAL_API bool SRAL_SetRateEx(int engine, uint64_t value);

	/**
 * @brief Get the current speech rate for the specified engine.
 * @param engine The engine to query.
 * @return The current speech rate for the engine.
 */

	SRAL_API uint64_t SRAL_GetRateEx(int engine);

	/**
 * @brief Get the count of available voices for the specified engine.
 * @param engine The engine to query.
 * @return The number of voices available for the speech engine.
 */

	SRAL_API uint64_t SRAL_GetVoiceCountEx(int engine);

	/**
 * @brief Get the name of a voice for the specified engine by its index.
 * @param engine The engine to query.
 * @param index The index of the voice.
 * @return A pointer to the name of the voice.
 */

	SRAL_API const char* SRAL_GetVoiceNameEx(int engine, uint64_t index);

	/**
 * @brief Set the currently selected voice for the specified engine by index.
 * @param engine The engine to set the voice for.
 * @param index The index of the voice to set.
 * @return true if the voice was set successfully, false otherwise.
 */

	SRAL_API bool SRAL_SetVoiceEx(int engine, uint64_t index);



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



#ifdef __cplusplus
}// extern "C"
#endif


#endif // SRAL_H_