package SRAL

// Engine represents bit flags identifying various accessibility engines,
// such as screen readers, speech synthesis engines, or accessibility frameworks.
type Engine int

const (
	// NoneEngine indicates no specific engine identified, or engine is unknown.
	NoneEngine Engine = 0
	// NVDAEngine — NonVisual Desktop Access (NVDA) for Windows.
	NVDAEngine Engine = Engine(1 << iota)
	// JAWSEngine — Job Access With Speech (JAWS) for Windows.
	JAWSEngine
	// ZDSREngine — Zhengdu Screen Reader (ZDSR) for Windows.
	ZDSREngine
	// NarratorEngine — Microsoft Narrator, the built-in screen reader for Windows.
	NarratorEngine
	// UIAEngine — Microsoft UI Automation (UIA) framework for Windows.
	UIAEngine
	// SAPIEngine — Microsoft Speech API (SAPI) for text-to-speech on Windows.
	SAPIEngine
	// SpeechDispatcherEngine — Speech Dispatcher, a common daemon for Linux systems.
	SpeechDispatcherEngine
	// VoiceOverEngine — Apple VoiceOver, the built-in screen reader on Apple platforms.
	VoiceOverEngine
	// NSSpeechEngine — Apple NSSpeechSynthesizer.
	NSSpeechEngine
	// AVSpeechEngine — AVFoundation Speech Synthesizer (AVSpeechSynthesizer) for Apple platforms.
	AVSpeechEngine
	// AllEngines is a bitmask of all supported engines.
	AllEngines Engine = NVDAEngine | JAWSEngine | ZDSREngine | NarratorEngine | UIAEngine | SAPIEngine | SpeechDispatcherEngine | NSSpeechEngine | VoiceOverEngine | AVSpeechEngine
	// InvalidEngine represents an error or uninitialized engine state.
	InvalidEngine Engine = -1
	// NoSpecifiedEngine is used for auto-selection of the engine.
	NoSpecifiedEngine Engine = -255
)

// Feature defines the features supported by various speech engines.
type Feature int

const (
	// NoneFeature indicates no features supported.
	NoneFeature Feature = 0
	// SpeechFeature indicates support for text-to-speech.
	SpeechFeature = Feature(1 << iota)
	// BrailleFeature indicates support for Braille output.
	BrailleFeature
	// SpeechRateFeature indicates support for adjusting speech rate.
	SpeechRateFeature
	// SpeechVolumeFeature indicates support for adjusting speech volume.
	SpeechVolumeFeature
	// SelectVoiceFeature indicates support for selecting different voices.
	SelectVoiceFeature
	// PauseSpeechFeature indicates support for pausing speech.
	PauseSpeechFeature
	// SSMLFeature indicates support for SSML tags.
	SSMLFeature
	// SpeakToMemoryFeature indicates support for synthesizing speech to a PCM buffer.
	SpeakToMemoryFeature
	// SpellingFeature indicates support for spelling out text.
	SpellingFeature
	// AllFeatures is a bitmask of all available engine features.
	AllFeatures = SpeechFeature | BrailleFeature | SpeechRateFeature | SpeechVolumeFeature | SelectVoiceFeature | PauseSpeechFeature | SSMLFeature | SpeakToMemoryFeature | SpellingFeature
)

// IsSupported checks if the feature set includes the specified feature.
func (f Feature) IsSupported(other Feature) bool {
	return (f & other) != 0
}

// EngineParam defines parameters that can be set or retrieved from a speech engine.
type EngineParam int

const (
	// SpeechRateParam — parameter for speech rate.
	SpeechRateParam EngineParam = EngineParam(iota)
	// SpeechVolumeParam — parameter for speech volume.
	SpeechVolumeParam
	// VoiceIndexParam — index of the current voice.
	VoiceIndexParam
	// VoicePropertiesParam — properties of the available voices.
	VoicePropertiesParam
	// VoiceCountParam — total number of available voices.
	VoiceCountParam
	// SymbolLevelParam — punctuation/symbol speaking level.
	SymbolLevelParam
	// SAPITrimThresholdParam — trim threshold specifically for SAPI.
	SAPITrimThresholdParam
	// EnableSpellingParam — toggles spelling mode.
	EnableSpellingParam
	// UseCharacterDescriptionsParam — toggles character descriptions.
	UseCharacterDescriptionsParam
	// NVDAIsControlExParam — specific parameter for NVDA control.
	NVDAIsControlExParam
)

// VoiceInfo contains information about a specific synthesizer voice.
type VoiceInfo struct {
	Index    int    // Index of the voice within the engine
	Name     string // Name of the voice
	Language string // Language code (e.g., "en-US")
	Gender   string // Gender of the voice
	Vendor   string // Vendor or developer of the voice
}

// PCMData represents raw audio data (PCM) generated when speaking to memory.
type PCMData struct {
	Buffer        []byte // Raw audio buffer
	Channels      int    // Number of audio channels
	SampleRate    int    // Sample rate in Hz
	BitsPerSample int    // Bit depth (bits per sample)
}
