# SRAL
Screen Reader Abstraction Library
## Description
SRAL is a cross-platform library for output text using speech engines.

## Platforms
SRAL is supported on Windows, MacOS and Linux platforms.



## Enumerations

### `SRAL_Engines`
This enumeration defines the available speech engines supported by the SRAL library. The values are:
- `ENGINE_NONE = 0`: No engine selected.
- `ENGINE_NVDA = 2`: NVDA screen reader.
- `ENGINE_SAPI = 4`: Microsoft SAPI5 speech engine.
- `ENGINE_JAWS = 8`: Jaws screen reader.
- `ENGINE_SPEECH_DISPATCHER = 16`: Speech Dispatcher engine.
- `ENGINE_UIA = 32`: Microsoft UI Automation provider.
- `ENGINE_AV_SPEECH = 64`: AVSpeech engine.
- `ENGINE_NARRATOR = 128`: Windows Narrator to UIAutomation redirector, if running.



### `SRAL_SupportedFeatures`
This enumeration defines the features supported by the various speech engines. The values are:
- `SUPPORTS_SPEECH = 128`: The engine supports speech output.
- `SUPPORTS_BRAILLE = 256`: The engine supports Braille output.
- `SUPPORTS_SPEECH_RATE = 512`: The engine supports setting the speech rate.
- `SUPPORTS_SPEECH_VOLUME = 1024`: The engine supports setting the speech volume.
- `SUPPORTS_SELECT_VOICE = 2048`: The engine supports selecting a specific voice.
- `SUPPORTS_PAUSE_SPEECH = 4096`: The engine supports pause and resume speech.


## Functions

### `SRAL_Speak(const char* text, bool interrupt)`
- **Description**: Speaks the given text.
- **Parameters**:
  - `text`: A pointer to the text string to be spoken.
  - `interrupt`: A flag indicating whether to interrupt the current speech.
- **Return Value**: `true` if speaking was successful, `false` otherwise.

### `SRAL_Braille(const char* text)`
- **Description**: Outputs the given text to a Braille display.
- **Parameters**:
  - `text`: A pointer to the text string to be output in Braille.
- **Return Value**: `true` if Braille output was successful, `false` otherwise.

### `SRAL_Output(const char* text, bool interrupt)`
- **Description**: Outputs the given text using all currently supported speech engine methods.
- **Parameters**:
  - `text`: A pointer to the text string to be output.
  - `interrupt`: A flag indicating whether to interrupt the current speech.
- **Return Value**: `true` if output was successful, `false` otherwise.

### `SRAL_StopSpeech(void)`
- **Description**: Stops speech if it is active.
- **Return Value**: `true` if speech was stopped successfully, `false` otherwise.

### `SRAL_PauseSpeech(void)`
- **Description**: Pause speech if it is active and the current engine supports this.
- **Return Value**: `true` if speech was paused successfully, `false` otherwise.

### `SRAL_ResumeSpeech(void)`
- **Description**: Resume speech if it was paused.
- **Return Value**: `true` if speech was resumed successfully, `false` otherwise.


### `SRAL_GetCurrentEngine(void)`
- **Description**: Gets the current speech engine in use.
- **Return Value**: The identifier of the current speech engine defined by the `SRAL_Engines` enumeration.

### `SRAL_GetEngineFeatures(int engine)`
- **Description**: Gets the features supported by the specified engine.
- **Parameters**:
  - `engine`: The identifier of the engine to query. Defaults to 0 (current engine).
- **Return Value**: An integer representing the features supported by the engine defined by the `SRAL_SupportedFeatures` enumeration.

### `SRAL_Initialize(int engines_exclude)`
- **Description**: Initializes the library and optionally excludes certain engines.
- **Parameters**:
  - `engines_exclude`: A bitmask specifying engines to exclude from initialization. Defaults to 0 (include all).
- **Return Value**: `true` if initialization was successful, `false` otherwise.

### `SRAL_Uninitialize(void)`
- **Description**: Uninitializes the library, freeing resources.

### `SRAL_SetVolume(uint64_t value)`
- **Description**: Sets the speech volume level, if the current speech engine supports this.
- **Parameters**:
  - `value`: The desired volume level.
- **Return Value**: `true` if the volume was set successfully, `false` otherwise.

### `SRAL_GetVolume(void)`
- **Description**: Gets the current speech volume level of the current speech engine.
- **Return Value**: The current volume level.

### `SRAL_SetRate(uint64_t value)`
- **Description**: Sets the speech rate, if the current engine supports this.
- **Parameters**:
  - `value`: The desired speech rate.
- **Return Value**: `true` if the speech rate was set successfully, `false` otherwise.

### `SRAL_GetRate(void)`
- **Description**: Gets the current speech rate of the current speech engine.
- **Return Value**: The current speech rate.

### `SRAL_GetVoiceCount(void)`
- **Description**: Gets the count of available voices supported by the current speech engine.
- **Return Value**: The number of available voices.

### `SRAL_GetVoiceName(uint64_t index)`
- **Description**: Gets the name of a voice by its index, if the current speech engine supports this.
- **Parameters**:
  - `index`: The index of a voice to get.
- **Return Value**: A pointer to the name of the voice.

### `SRAL_SetVoice(uint64_t index)`
- **Description**: Sets the currently selected voice by index, if the current speech engine supports this.
- **Parameters**:
  - `index`: The index of a voice to set.
- **Return Value**: `true` if the voice was set successfully, `false` otherwise.

### Extended Functions

The library also provides extended functions to perform operations with specific speech engines. These functions follow the same naming convention as the regular functions, but with the addition of the `Ex` suffix. For example, `SRAL_SpeakEx`, `SRAL_BrailleEx`, `SRAL_OutputEx`, etc.

### `SRAL_IsInitialized(void)`
- **Description**: Checks if the library has been initialized.
- **Return Value**: `true` if the library is initialized, `false` otherwise.

### `SRAL_Delay(int time)`
- **Description**: Delayes the next speech or output operation by a given time.


## Example
C
```
#include <stdio.h>
#include <SRAL.h>

int main(void) {
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

```

## Additional info
For [NVDA](https://github.com/nvaccess/nvda) screen reader, you need to download the [Controller Client](https://www.nvaccess.org/files/nvda/releases/stable/) for this.
