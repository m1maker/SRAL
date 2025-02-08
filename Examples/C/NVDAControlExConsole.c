#include "../../Dep/nvda_control.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>  // For _kbhit() and _getch()

BOOL g_Running;
HANDLE g_Nvda;

void on_exit(void) {
	printf("Exiting...\n");
	g_Running = FALSE;
}

BOOL WINAPI ConsoleHandler(DWORD signal) {
	switch (signal) {
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
			on_exit();
			return TRUE; // Indicate that we handled the event
		default:
			return FALSE; // Pass on to default handler
	}
}

int main(void) {
	char command[64000];
	g_Nvda = nvda_connect();
	if (g_Nvda == INVALID_HANDLE_VALUE) {
		printf("Failed to connect to NVDA named pipe.\n");
		return -1;
	}

	// Set console to use UTF-8
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	printf("Welcome to NVDA Controller Extended Console!\nTo find commands and expected arguments, see the NVDAControlEx addon documentation\n");
	SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	g_Running = TRUE;

	while (g_Running) {
		// Print prompt
		putchar('>');
		
		// Clear the command buffer
		memset(command, 0, sizeof(command));
		
		// Read input character by character
		int i = 0;
		while (g_Running && i < sizeof(command) - 1) {
			if (_kbhit()) { // Check if a key has been pressed
				char ch = _getch(); // Get the character without echoing it
				if (ch == '\r') { // Enter key
					break; // End input on Enter
				} else if (ch == 8) { // Backspace key
					if (i > 0) { // Check if there's something to delete
						i--; // Move index back
						command[i] = '0'; // Null-terminate the string
						// Move cursor back and overwrite with space, then move back again
						printf("\b \b"); // Backspace, print space, backspace again
					}
				} else if (ch <= 0) { // Special keys
					_getch(); // Read the next character to complete the sequence
					continue; // Ignore the special key
					MessageBeep(MB_ICONERROR);
				} else {
					command[i++] = ch; // Store character in command buffer
					putchar(ch); // Echo the character
				}
			}
			Sleep(5); // Sleep briefly to avoid busy-waiting
		}

		command[i] = '0'; // Null-terminate the string

		if (g_Running && strlen(command) > 0) { // Check if we should still be running and if there's a command
			if (nvda_send_command(g_Nvda, command) == -1) {
				printf("Failed to send command %s\n", command);
			}
			printf("\r\n"); // Move to a new line (next command)
		}
	}
	nvda_disconnect(g_Nvda);
	return 0;
}

