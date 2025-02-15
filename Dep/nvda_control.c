/*
 * NVDA Control Source File
 *
 * Copyright (c) 2025 [m1maker]
 */

#include "nvda_control.h"

#ifdef __cplusplus
extern "C" {
#endif

static HANDLE g_hNvda = INVALID_HANDLE_VALUE;


// Function to escape quotes in the text
static void escape_quotes(const char* input, char* output, size_t output_size) {
    size_t i, j = 0;
    for (i = 0; input[i] != '\0' && j < output_size - 1; i++) {
        if (input[i] == '"') {
            // Escape the quote
            if (j < output_size - 2) { // Check if there's space for escaped quote
                output[j++] = '\\';
                output[j++] = '"';
            }
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0'; // Null-terminate the output string
}


// Connects to the NVDA named pipe
int nvda_connect(void) {
    if (g_hNvda != INVALID_HANDLE_VALUE) {
        nvda_disconnect();
    }

    g_hNvda = CreateFileW(
        NVDA_PIPE_NAME,          // Pipe name
        GENERIC_READ | GENERIC_WRITE, // Read and write access
        0,                       // No sharing
        NULL,                    // Default security attributes
        OPEN_EXISTING,           // Opens existing pipe
        0,                       // Default attributes
        NULL                     // No template file
    );
    return g_hNvda != INVALID_HANDLE_VALUE ? 0 : -1;
}

// Disconnects from the NVDA named pipe
void nvda_disconnect(void) {
    if (g_hNvda != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(g_hNvda);
        CloseHandle(g_hNvda);
    }
    g_hNvda = INVALID_HANDLE_VALUE;
}

// Sends a command to the NVDA named pipe
int nvda_send_command(const char* command) {
    if (g_hNvda == INVALID_HANDLE_VALUE) {
        nvda_connect();  // Reconnect if the pipe is invalid
        if (g_hNvda == INVALID_HANDLE_VALUE) {
            return -1;
        }
    }

    DWORD bytesWritten;
    BOOL result = WriteFile(
        g_hNvda,                   // Pipe handle
        command,                 // Command to send
        (DWORD)strlen(command),  // Length of command
        &bytesWritten,           // Bytes written
        NULL                     // Not overlapped
    );

    if (!result) {
        // If the write fails, close the pipe and reconnect
        nvda_disconnect();
        nvda_connect();
        if (g_hNvda == INVALID_HANDLE_VALUE) {
            return -1;
        }
        return nvda_send_command(command);
    }

    return 0;
}

// Sends a "speak" command to NVDA
int nvda_speak(const char* text, int symbol_level) {
    char escaped_text[64000];
    escape_quotes(text, escaped_text, sizeof(escaped_text));


    char command[64000];
    snprintf(command, sizeof(command), "speak \"%s\" 0 %d", escaped_text, symbol_level);
    return nvda_send_command(command);
}

// Sends a "speakSpelling" command to NVDA
int nvda_speak_spelling(const char* text, const char* locale, int use_character_descriptions) {
    char escaped_text[64000];
    escape_quotes(text, escaped_text, sizeof(escaped_text));


    char command[64000];
    snprintf(command, sizeof(command), "speakSpelling \"%s\" \"%s\" %d", escaped_text, locale, use_character_descriptions);
    return nvda_send_command(command);
}

// Sends a "speakSsml" command to NVDA
int nvda_speak_ssml(const char* ssml, int symbol_level) {
    char escaped_text[64000];
    escape_quotes(ssml, escaped_text, sizeof(escaped_text));


    char command[64000];
    snprintf(command, sizeof(command), "speakSsml \"%s\" 0 %d", escaped_text, symbol_level);
    return nvda_send_command(command);
}

// Sends a "pauseSpeech" command to NVDA
int nvda_pause_speech(int pause) {
    char command[256];
    snprintf(command, sizeof(command), "pauseSpeech %d", pause);
    return nvda_send_command(command);
}

// Sends a "cancelSpeech" command to NVDA
int nvda_cancel_speech(void) {
    return nvda_send_command("cancelSpeech");
}

// Sends a "braille" command to NVDA
int nvda_braille(const char* text) {
    char escaped_text[64000];
    escape_quotes(text, escaped_text, sizeof(escaped_text));


    char command[64000];
    snprintf(command, sizeof(command), "braille \"%s\"", escaped_text);
    return nvda_send_command(command);
}

// Function to check if NVDA is active
int nvda_active(void) {
    if (g_hNvda == INVALID_HANDLE_VALUE) {
        return -1;  // Pipe is not connected
    }

    static const char* pingCommand = "active";
    DWORD bytesWritten;
    BOOL result = WriteFile(
        g_hNvda,
        pingCommand,
        (DWORD)strlen(pingCommand),
        &bytesWritten,
        NULL
    );

    if (!result || bytesWritten != strlen(pingCommand)) {
        nvda_disconnect();
        return -1;
    }

    char buffer[5];
    result = ReadFile(
        g_hNvda,
        buffer,
        sizeof(buffer) - 1,
        NULL,
        NULL
    );

    if (!result) {
        return -1;
    }

    return strcmp(buffer, "NVDA") == 0 ? 0 : -1;
}


#ifdef __cplusplus
}
#endif

