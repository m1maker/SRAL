#include "nvda_control.h"

#ifdef __cplusplus
extern "C" {
#endif


// Connects to the NVDA named pipe
HANDLE nvda_connect() {
    HANDLE hPipe = CreateFileW(
        NVDA_PIPE_NAME,          // Pipe name
        GENERIC_READ | GENERIC_WRITE, // Read and write access
        0,                       // No sharing
        NULL,                    // Default security attributes
        OPEN_EXISTING,           // Opens existing pipe
        0,                       // Default attributes
        NULL                     // No template file
    );
    return hPipe;
}

// Disconnects from the NVDA named pipe
void nvda_disconnect(HANDLE hPipe) {
    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
    }
}

// Sends a command to the NVDA named pipe
int nvda_send_command(HANDLE hPipe, const char* command) {
    if (hPipe == INVALID_HANDLE_VALUE) {
        return -1;
    }

    DWORD bytesWritten;
    BOOL result = WriteFile(
        hPipe,                   // Pipe handle
        command,                 // Command to send
        (DWORD)strlen(command),  // Length of command
        &bytesWritten,           // Bytes written
        NULL                     // Not overlapped
    );

    if (!result) {
        return -1;
    }

    return 0;
}

// Sends a "speak" command to NVDA
int nvda_speak(HANDLE hPipe, const char* text, int symbol_level) {
    char command[64000];
    snprintf(command, sizeof(command), "speak \"%s\" 0 %d", text, symbol_level);
    return nvda_send_command(hPipe, command);
}

// Sends a "speakSpelling" command to NVDA
int nvda_speak_spelling(HANDLE hPipe, const char* text, const char* locale, int use_character_descriptions) {
    char command[64000];
    snprintf(command, sizeof(command), "speakSpelling \"%s\" \"%s\" %d", text, locale, use_character_descriptions);
    return nvda_send_command(hPipe, command);
}

// Sends a "speakSsml" command to NVDA
int nvda_speak_ssml(HANDLE hPipe, const char* ssml, int symbol_level) {
    char command[64000];
    snprintf(command, sizeof(command), "speakSsml \"%s\" 0 %d", ssml, symbol_level);
    return nvda_send_command(hPipe, command);
}

// Sends a "pauseSpeech" command to NVDA
int nvda_pause_speech(HANDLE hPipe, int pause) {
    char command[256];
    snprintf(command, sizeof(command), "pauseSpeech %d", pause);
    return nvda_send_command(hPipe, command);
}

// Sends a "cancelSpeech" command to NVDA
int nvda_cancel_speech(HANDLE hPipe) {
    return nvda_send_command(hPipe, "cancelSpeech");
}

#ifdef __cplusplus
}
#endif

