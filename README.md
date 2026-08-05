# SRAL (Screen Reader Abstraction Library)

> [!NOTE]
> Consider using **[prism](https://github.com/ethindp/prism)** instead of SRAL.
> Its architecture is more correct, it supports more engines (except NSSpeech and BRLTTY, but BRLTTY specifically is not needed when Orca is available, but I don't know about legacy NSSpeech).
> It also supports the creation of your own custom backends via vtable.
> For more information, see the link above.
> Thanks to everyone who has ever contributed to the SRAL project. I'm not doing this anymore because I don't see the point in developing it, at least because of architectural problems and my lack of development experience at that time.

SRAL is a cross-platform library designed to provide a unified interface for outputting speech and Braille via assistive technologies. It abstracts the complexities of various screen readers and speech APIs, allowing developers to implement accessibility features once and have them work across multiple engines and platforms.

## 🌍 Language Note

*Note: I am not a native English speaker. All documentation, including this README, has been written with AI assistance to ensure clarity and proper grammar while maintaining the technical accuracy of the project descriptions.*

---

## 🚀 Features

* **Unified API**: Speak text, pause, stop, and resume speech using a single interface.
* **Braille Support**: Direct output to Braille displays.
* **Parameter Control**: Adjust speech rate, volume, and voices (where supported by the engine).
* **Engine Prioritization**: Automatic detection of active screen readers with fallback to system Speech APIs.
* **Advanced Audio**: Support for "Speak to Memory" (PCM buffer output) and SSML tags.
* **Keyboard Hooks**: Optional global hooks for interrupting (Ctrl) or pausing (Shift) speech.

## 🛠 Supported Engines & Platforms

SRAL supports Windows, macOS, iOS, Android, and Linux.

| Category | Supported Engines |
| --- | --- |
| **Windows Screen Readers** | NVDA, JAWS, ZDSR, Microsoft Narrator |
| **Windows Frameworks** | Microsoft UI Automation (UIA) |
| **macOS** | VoiceOver, NSSpeech, AVFoundation (AVSpeech) |
| **iOS** | VoiceOver, AVFoundation (AVSpeech) |
| **Android** | Android TextToSpeech, Android AccessibilityManager (TalkBack etc.) |
| **Linux** | Speech Dispatcher |
| **General APIs** | Microsoft SAPI (Windows), BRLTTY (Braille) |

---

## ❓ Why use SRAL?

SRAL is ideal for making **applications or games** accessible to blind or visually impaired users.

> [!IMPORTANT]
> **Note on Accessibility**: SRAL is for direct speech/braille output. It is **not** a UI accessibility bridge. If you need to make your GUI elements (buttons, menus, etc.) visible to screen readers, please use [AccessKit](https://github.com/AccessKit/accesskit)

---

## ⚙️ How it Works

### Initialization & Priorities

When you initialize the library, SRAL loads all available and supported engines.

* **Standard Functions (`SRAL_Speak`, `SRAL_StopSpeech`, etc.)**: These automatically choose the best engine based on priority. The priority order is:
1. **Screen Readers** (highest)
2. **Speech APIs** (SAPI, Speech Dispatcher)
3. **A11y Providers** (UIA)


* **Extended Functions (`SRAL_SpeakEx`, etc.)**: These allow you to manually target a specific engine, bypassing the automatic priority logic.

### Building the Project

SRAL uses CMake and can be built as either a static or dynamic library.

**Linux Requirements:**
You must install the following packages: `libspeechd-dev`, `libbrlapi-dev`, and `brltty`.

**Build Commands:**

```bash
cmake . -B build
cmake --build build --config Release

```

This will also generate an executable test utility to verify SRAL functionality on your system.

---

## 💻 Usage

### C/C++ Integration

To use SRAL, include the header and link the library.

**Static Linking (Windows):**
You must define `SRAL_STATIC` before including the header.

```c
#define SRAL_STATIC
#include <SRAL.h>

```

**C++ Wrapper:**
For C++ developers, a convenient inline wrapper is available in `Include/SRAL.hpp`.

### NVDA Support

* **NVDAControlEx**: SRAL supports the [NVDAControlEx](https://github.com/m1maker/NVDAControlEx) add-on for extended management.
* **Controller Client**: For standard NVDA support, download the [Controller Client](https://www.nvaccess.org/files/nvda/releases/stable/). Please note that **Version 1 is not supported**.

### Language Bindings

* **Python**: Currently supported and available in the repository.
* **LUA**: Bindings are currently in preparation and coming soon.

---

## 📄 API Overview (Snippet)

For full documentation, see `Include/SRAL.h`.

```c
// Initialize library and exclude specific engines if needed
bool SRAL_Initialize(int engines_exclude);

// Speak text using the best available engine
bool SRAL_Speak(const char* text, bool interrupt);

// Output text to Braille display
bool SRAL_Braille(const char* text);

// Check if an engine is currently speaking
bool SRAL_IsSpeaking(void);

// Categorized engine bitmasks (use with SRAL_SetEnginesExclude)
int  SRAL_GetTTSEngines(void);
int  SRAL_GetAssistiveTechEngines(void);
```

### Routing only through assistive tech (with optional in-app TTS)

If your application should always speak through the user's assistive
technology, but only fall back to platform TTS when the user has
explicitly enabled it, exclude the TTS engines by default:

```c
bool tts_option_enabled = /* your app's setting */;
SRAL_SetEnginesExclude(tts_option_enabled ? 0 : SRAL_GetTTSEngines());
SRAL_Speak("hello", true);  // routes to AT always; to TTS only if opted in
```
