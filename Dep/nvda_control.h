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


/**
 * Connects to the NVDA named pipe.
 * Returns a handle to the pipe if successful, or INVALID_HANDLE_VALUE on failure.
 */
HANDLE nvda_connect(void);

/**
 * Disconnects from the NVDA named pipe.
 * @param hPipe Handle to the pipe.
 */
void nvda_disconnect(HANDLE hPipe);

/**
 * Sends a command to the NVDA named pipe.
 * @param hPipe Handle to the pipe.
 * @param command The command to send.
 * @return 0 on success, -1 on failure.
 */
int nvda_send_command(HANDLE hPipe, const char* command);

/**
 * Sends a "speak" command to NVDA.
 * @param hPipe Handle to the pipe.
 * @param text The text to speak.
 * @return 0 on success, -1 on failure.
 */
int nvda_speak(HANDLE hPipe, const char* text);

/**
 * Sends a "speakSpelling" command to NVDA.
 * @param hPipe Handle to the pipe.
 * @param text The text to spell.
 * @return 0 on success, -1 on failure.
 */
int nvda_speak_spelling(HANDLE hPipe, const char* text);

/**
 * Sends a "speakSsml" command to NVDA.
 * @param hPipe Handle to the pipe.
 * @param ssml The SSML text to speak.
 * @return 0 on success, -1 on failure.
 */
int nvda_speak_ssml(HANDLE hPipe, const char* ssml);

/**
 * Sends a "pauseSpeech" command to NVDA.
 * @param hPipe Handle to the pipe.
 * @param pause 1 to pause, 0 to unpause.
 * @return 0 on success, -1 on failure.
 */
int nvda_pause_speech(HANDLE hPipe, int pause);

/**
 * Sends a "cancelSpeech" command to NVDA.
 * @param hPipe Handle to the pipe.
 * @return 0 on success, -1 on failure.
 */
int nvda_cancel_speech(HANDLE hPipe);

#ifdef __cplusplus
}
#endif


#endif // NVDA_CONTROL_H
