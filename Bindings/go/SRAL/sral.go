// Package SRAL (Screen Reader Abstraction Library) provides a unified interface
// for interacting with various speech engines.
// It abstracts the differences between multiple speech engines, allowing developers to
// implement accessibility features in their applications without needing to handle the
// specifics of each engine.
package SRAL

/*
#include "SRAL.h"
*/
import "C"

import (
	"time"
)

// Speak speaks the given text using the current engine.
// The interrupt flag indicates whether to interrupt the current speech.
// Returns true if speaking was successful, false otherwise.
func Speak(text string, interrupt bool) bool {
	return speak(NoSpecifiedEngine, text, interrupt)
}

// SpeakToMemory speaks the given text into memory using the current engine.
// Returns a PCMData pointer containing the buffer and its properties if successful.
func SpeakToMemory(text string) (*PCMData, error) {
	return speakToMemory(NoSpecifiedEngine, text)
}

// SpeakSsml speaks the given valid SSML string using the current engine.
// The interrupt flag indicates whether to interrupt the current speech.
// Returns true if speaking was successful, false otherwise.
func SpeakSsml(ssml string, interrupt bool) bool {
	return speakSsml(NoSpecifiedEngine, ssml, interrupt)
}

// Braille outputs the given text to a Braille display using the current engine.
// Returns true if Braille output was successful, false otherwise.
func Braille(text string) bool {
	return braille(NoSpecifiedEngine, text)
}

// Output outputs text using all currently supported speech engine methods.
// The interrupt flag indicates whether to interrupt speech.
// Returns true if output was successful, false otherwise.
func Output(text string, interrupt bool) bool {
	return output(NoSpecifiedEngine, text, interrupt)
}

// StopSpeech stops speech if it is active on the current engine.
// Returns true if speech was stopped successfully, false otherwise.
func StopSpeech() bool {
	return stopSpeech(NoSpecifiedEngine)
}

// PauseSpeech pauses speech if it is active and the current speech engine supports this.
// Returns true if speech was paused successfully, false otherwise.
func PauseSpeech() bool {
	return pauseSpeech(NoSpecifiedEngine)
}

// ResumeSpeech resumes speech if it was paused and the current speech engine supports this.
// Returns true if speech was resumed successfully, false otherwise.
func ResumeSpeech() bool {
	return resumeSpeech(NoSpecifiedEngine)
}

// IsSpeaking returns true if the current engine is currently speaking, false otherwise.
func IsSpeaking() bool {
	return isSpeaking(NoSpecifiedEngine)
}

// GetCurrentEngine returns the identifier of the current speech engine in use.
func GetCurrentEngine() Engine {
	return Engine(C.SRAL_GetCurrentEngine())
}

// GetEngineFeatures returns the features supported by the specified engine.
func GetEngineFeatures(engine Engine) Feature {
	return Feature(C.SRAL_GetEngineFeatures(C.int(engine)))
}

// GetEngineParameter retrieves a parameter for the specified speech engine.
// The out parameter must be a pointer to int, bool, or a slice of VoiceInfo.
// Returns true if the parameter was retrieved successfully.
func GetEngineParameter(engine Engine, param EngineParam, out any) bool {
	switch v := out.(type) {
	case *int:
		return getIntParam(engine, param, v)
	case *bool:
		return getBoolParam(engine, param, v)
	case *[]VoiceInfo:
		return getVoicesParam(engine, param, v)
	default:
		return false
	}
}

// SetEngineParameter sets a parameter for the specified speech engine.
// The value can be an int or a bool.
// Returns true if the parameter was set successfully.
func SetEngineParameter(engine Engine, param EngineParam, value any) bool {
	switch v := value.(type) {
	case int:
		return setIntParam(engine, param, v)
	case bool:
		return setBoolParam(engine, param, v)
	default:
		return false
	}
}

// Initialize initializes the library and optionally excludes certain engines
// from auto-update via a bitmask. Returns true if successful.
func Initialize(excluded_engines Engine) bool {
	return bool(C.SRAL_Initialize(C.int(excluded_engines)))
}

// Uninitialize uninitializes the library, freeing resources.
func Uninitialize() {
	C.SRAL_Uninitialize()
}

// IsInitialized checks if the library has been initialized.
func IsInitialized() bool {
	return bool(C.SRAL_IsInitialized())
}

// SpeakEx speaks the given text with the specified engine.
// Returns true if speaking was successful.
func SpeakEx(engine Engine, text string, interrupt bool) bool {
	return speak(engine, text, interrupt)
}

// SpeakToMemoryEx speaks the given text into memory with the specified engine.
func SpeakToMemoryEx(engine Engine, text string) (*PCMData, error) {
	return speakToMemory(engine, text)
}

// SpeakSsmlEx speaks the given SSML string with the specified engine.
func SpeakSsmlEx(engine Engine, ssml string, interrupt bool) bool {
	return speakSsml(engine, ssml, interrupt)
}

// BrailleEx outputs text to a Braille display using the specified engine.
func BrailleEx(engine Engine, text string) bool {
	return braille(engine, text)
}

// OutputEx outputs text using the specified engine.
func OutputEx(engine Engine, text string, interrupt bool) bool {
	return output(engine, text, interrupt)
}

// StopSpeechEx stops speech for the specified engine.
func StopSpeechEx(engine Engine) bool {
	return stopSpeech(engine)
}

// PauseSpeechEx pauses speech for the specified engine.
func PauseSpeechEx(engine Engine) bool {
	return pauseSpeech(engine)
}

// ResumeSpeechEx resumes speech for the specified engine.
func ResumeSpeechEx(engine Engine) bool {
	return resumeSpeech(engine)
}

// IsSpeakingEx returns true if the specified engine is currently speaking.
func IsSpeakingEx(engine Engine) bool {
	return isSpeaking(engine)
}

// Delay delays the next speech or output operation by the given duration.
func Delay(t time.Duration) {
	cMs := C.int(t.Milliseconds())
	C.SRAL_Delay(cMs)
}

// RegisterKeyboardHooks installs global keyboard hooks for speech interruption (Ctrl)
// and pause (Shift) for engines like SAPI or SpeechDispatcher.
// Returns true if hooks were successfully installed.
func RegisterKeyboardHooks() bool {
	return bool(C.SRAL_RegisterKeyboardHooks())
}

// UnregisterKeyboardHooks uninstalls the speech interruption and pause keyboard hooks.
func UnregisterKeyboardHooks() {
	C.SRAL_UnregisterKeyboardHooks()
}

// GetAvailableEngines returns a bitmask of all available engines for the current platform.
func GetAvailableEngines() Engine {
	engines := C.SRAL_GetAvailableEngines()
	return Engine(engines)
}

// GetActiveEngines returns a bitmask of all active engines that can be used.
func GetActiveEngines() Engine {
	engines := C.SRAL_GetActiveEngines()
	return Engine(engines)
}

// GetEngineName returns the name of the specified engine.
func GetEngineName(engine Engine) string {
	cName := C.SRAL_GetEngineName(C.int(engine))
	name := C.GoString(cName)
	return name
}

// SetEnginesExclude sets a bitmask of engines to be excluded from auto-update.
func SetEnginesExclude(excluded_engines Engine) bool {
	return bool(C.SRAL_SetEnginesExclude(C.int(excluded_engines)))
}

// GetEnginesExclude returns the bitmask of engines excluded from auto-update.
// Returns InvalidEngine if the library is not initialized.
func GetEnginesExclude() Engine {
	excluded_engines := int(C.SRAL_GetEnginesExclude())
	if excluded_engines == -1 {
		return InvalidEngine
	}
	return Engine(excluded_engines)
}
