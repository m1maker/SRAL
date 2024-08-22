#include <stdio.h>
#include <SRAL.h>

int main() {
    // Initialize the SRAL library
    if (!SRAL_Initialize(0)) {
        printf("Failed to initialize SRAL library.\n");
        return 1;
    }

    // Speak some text
    if (!SRAL_Speak("Hello, world!", false)) {
        printf("Failed to speak text.\n");
    }

    // Output text to a Braille display
    if (!SRAL_Braille("This is Braille output.")) {
        printf("Failed to output to Braille display.\n");
    }

    // Uninitialize the SRAL library
    SRAL_Uninitialize();

    return 0;
}
