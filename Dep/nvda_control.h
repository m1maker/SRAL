/*
 * NVDA Control Header File
 *
 * Copyright (c) 2025 [m1maker]
 * This header file defines the interface for communicating with the NVDA (NonVisual Desktop Access)
 * screen reader via a named pipe
 *
 * Prerequisites:
 * - The NVDAControlEx add-on must be installed in NVDA for this interface to function correctly.
 * https://github.com/m1maker/NVDAControlEx
 *   This add-on enhances the NVDA API, allowing for more advanced control and communication features.
 *
 * Usage:
 * To use this API, include this header file in your source code and link against the nvda_control.c
 * implementation that handles the actual communication with NVDA. Ensure that the NVDAControlEx add-on
 * is installed and enabled.
 */


#ifndef NVDA_CONTROL_H
#define NVDA_CONTROL_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif



// Named pipe name
#define NVDA_PIPE_NAME L"\\\\.\\pipe\\NVDAControlPipe"

// Symbol level enumeration
enum nvda_symbol_level
{
    NVDA_SYMBOL_LEVEL_NONE	= 0,
    NVDA_SYMBOL_LEVEL_SOME	= 100,
    NVDA_SYMBOL_LEVEL_MOST	= 200,
    NVDA_SYMBOL_LEVEL_ALL	= 300,
    NVDA_SYMBOL_LEVEL_CHAR	= 1000,
    NVDA_SYMBOL_LEVEL_UNCHANGED	= -1
};



/**
 * Connects to the NVDA named pipe.
 * Returns 0 if successful, or -1 on failure.
 */
int nvda_connect(void);

/**
 * Disconnects from the NVDA named pipe.
 */
void nvda_disconnect(void);

/**
 * Sends a command to the NVDA named pipe.
 * @param command The command to send.
 * @return 0 on success, -1 on failure.
 */
int nvda_send_command(const char* command);

/**
 * Sends a "speak" command to NVDA.
 * @param text The text to speak.
 * @param symbol_level Punctuation level for speech.
 * @return 0 on success, -1 on failure.
 */
int nvda_speak(const char* text, int symbol_level);

/**
 * Sends a "speakSpelling" command to NVDA.
 * @param text The text to spell.
 * @param locale The locale for speech.
 * @param use_character_descriptions Describe each spoken character.
 * @return 0 on success, -1 on failure.
 */
int nvda_speak_spelling(const char* text, const char* locale, int use_character_descriptions);

/**
 * Sends a "speakSsml" command to NVDA.
 * @param ssml The SSML text to speak.
 * @param symbol_level Punctuation level for speech.
 * @return 0 on success, -1 on failure.
 */
int nvda_speak_ssml(const char* ssml, int symbol_level);

/**
 * Sends a "pauseSpeech" command to NVDA.
 * @param pause 1 to pause, 0 to unpause.
 * @return 0 on success, -1 on failure.
 */
int nvda_pause_speech(int pause);

/**
 * Sends a "cancelSpeech" command to NVDA.
 * @return 0 on success, -1 on failure.
 */
int nvda_cancel_speech(void);

/**
 * Sends a "braille" command to NVDA.
 * @param text The text to show on braille display.
 * @return 0 on success, -1 on failure.
 */
int nvda_braille(const char* text);

/**
 * Check that NVDA is active.
 * Returns 0 if successful, or -1 on failure.
 */
int nvda_active(void);

#ifdef __cplusplus
}
#endif


#endif // NVDA_CONTROL_H
