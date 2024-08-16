# SRAL
Screen Reader Abstraction Library
## Description
SRAL is a cross-platform library for output text using speech engines, such as screen readers or SAPI for Windows or Speech Dispatcher for Linux.

## Platforms
SRAL is supported on Windows and Linux platforms.


## Example
C
```
#include "SRAL.h"

int main(void) {
    // Initialize SRAL
    if (!SRAL_Initialize(0, 0)) {
        printf("Failed to initialize SRAL\n");
        return 1;
    }

    // Get the current engine
    int currentEngine = SRAL_GetCurrentEngine();
    printf("Current engine: %d\n", currentEngine);

    // Get the supported features
    int engineFeatures = SRAL_GetEngineFeatures();
    printf("Supported features: %d\n", engineFeatures);

    // Check if speech is supported
    if (engineFeatures & SUPPORTS_SPEECH) {
        // Speak some text
        SRAL_Speak("Hello, world!", false);
    }

    // Check if braille is supported
    if (engineFeatures & SUPPORTS_BRAILLE) {
        // Output text to braille
        SRAL_Braille("Hello, world!");
    }

    // Set the volume
    SRAL_SetVolume(50);
    uint64_t volume = SRAL_GetVolume();
    printf("Volume: %llu\n", volume);

    // Set the speech rate
    SRAL_SetRate(75);
    uint64_t rate = SRAL_GetRate();
    printf("Rate: %llu\n", rate);

    // Stop the speech
    SRAL_StopSpeech();

    // Uninitialize SRAL
    SRAL_Uninitialize();

    return 0;
}
```
