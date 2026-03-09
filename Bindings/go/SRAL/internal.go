package SRAL

/*
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "SRAL.h"
*/
import "C"

import (
	"fmt"
	"unsafe"
)

func speak(engine Engine, text string, interrupt bool) bool {
	cText := C.CString(text)
	defer C.free(unsafe.Pointer(cText))
	if engine == NoSpecifiedEngine {
		return bool(C.SRAL_Speak(cText, C.bool(interrupt)))
	}
	return bool(C.SRAL_SpeakEx(C.int(engine), cText, C.bool(interrupt)))
}

func speakToMemory(engine Engine, text string) (*PCMData, error) {
	cText := C.CString(text)
	defer C.free(unsafe.Pointer(cText))
	var (
		bufSize    C.uint64_t
		channels   C.int
		sampleRate C.int
		bits       C.int
	)

	var ptr unsafe.Pointer
	if engine == NoSpecifiedEngine {
		ptr = C.SRAL_SpeakToMemory(cText, &bufSize, &channels, &sampleRate, &bits)
	} else {
		ptr = C.SRAL_SpeakToMemoryEx(C.int(engine), cText, &bufSize, &channels, &sampleRate, &bits)
	}
	if ptr == nil {
		return nil, fmt.Errorf("failed to synthesize speech to memory")
	}
	defer C.SRAL_free(ptr)
	data := C.GoBytes(ptr, C.int(bufSize))
	return &PCMData{
		Buffer:        data,
		Channels:      int(channels),
		SampleRate:    int(sampleRate),
		BitsPerSample: int(bits),
	}, nil
}

func speakSsml(engine Engine, ssml string, interrupt bool) bool {
	cText := C.CString(ssml)
	defer C.free(unsafe.Pointer(cText))
	if engine == NoSpecifiedEngine {
		return bool(C.SRAL_SpeakSsml(cText, C.bool(interrupt)))
	}
	return bool(C.SRAL_SpeakSsmlEx(C.int(engine), cText, C.bool(interrupt)))
}

func braille(engine Engine, text string) bool {
	cText := C.CString(text)
	defer C.free(unsafe.Pointer(cText))
	if engine == NoSpecifiedEngine {
		return bool(C.SRAL_Braille(cText))
	}
	return bool(C.SRAL_BrailleEx(C.int(engine), cText))
}

func output(engine Engine, text string, interrupt bool) bool {
	cText := C.CString(text)
	defer C.free(unsafe.Pointer(cText))
	if engine == NoSpecifiedEngine {
		return bool(C.SRAL_Output(cText, C.bool(interrupt)))
	}
	return bool(C.SRAL_OutputEx(C.int(engine), cText, C.bool(interrupt)))
}

func stopSpeech(engine Engine) bool {
	if engine == NoSpecifiedEngine {
		return bool(C.SRAL_StopSpeech())
	}
	return bool(C.SRAL_StopSpeechEx(C.int(engine)))
}

func pauseSpeech(engine Engine) bool {
	if engine == NoSpecifiedEngine {
		return bool(C.SRAL_PauseSpeech())
	}
	return bool(C.SRAL_PauseSpeechEx(C.int(engine)))
}

func resumeSpeech(engine Engine) bool {
	if engine == NoSpecifiedEngine {
		return bool(C.SRAL_ResumeSpeech())
	}
	return bool(C.SRAL_ResumeSpeechEx(C.int(engine)))
}

func isSpeaking(engine Engine) bool {
	if engine == NoSpecifiedEngine {
		return bool(C.SRAL_IsSpeaking())
	}
	return bool(C.SRAL_IsSpeakingEx(C.int(engine)))
}

func getIntParam(e Engine, p EngineParam, out *int) bool {
	var cVal C.int
	ptr := unsafe.Pointer(&cVal)
	if !bool(C.SRAL_GetEngineParameter(C.int(e), C.int(p), ptr)) {
		return false
	}
	*out = int(cVal)
	return true
}

func getBoolParam(e Engine, p EngineParam, out *bool) bool {
	var val int = 0
	ptr := unsafe.Pointer(&val)
	if !bool(C.SRAL_GetEngineParameter(C.int(e), C.int(p), ptr)) {
		return false
	}
	if val == 0 {
		*out = false
	} else {
		*out = true
	}
	return true
}

func getVoicesParam(e Engine, p EngineParam, out *[]VoiceInfo) bool {
	var count int
	if !getIntParam(e, VoiceCountParam, &count) {
		return false
	}
	size := C.size_t(count) * C.size_t(unsafe.Sizeof(C.SRAL_VoiceInfo{}))
	ptr := C.SRAL_malloc(size)
	if ptr == nil {
		return false
	}
	defer C.SRAL_free(ptr)
	if !bool(C.SRAL_GetEngineParameter(C.int(e), C.int(p), ptr)) {
		return false
	}

	cVoices := unsafe.Slice((*C.SRAL_VoiceInfo)(ptr), count)
	res := make([]VoiceInfo, count)
	for i := range cVoices {
		res[i] = VoiceInfo{
			Index:    int(cVoices[i].index),
			Name:     C.GoString(cVoices[i].name),
			Language: C.GoString(cVoices[i].language),
			Gender:   C.GoString(cVoices[i].gender),
			Vendor:   C.GoString(cVoices[i].vendor),
		}
	}
	*out = res
	return true
}

func setIntParam(e Engine, p EngineParam, val int) bool {
	cVal := C.int(val)
	return bool(C.SRAL_SetEngineParameter(C.int(e), C.int(p), unsafe.Pointer(&cVal)))
}

func setBoolParam(e Engine, p EngineParam, val bool) bool {
	var valAsNum int = 0
	if val {
		valAsNum = 1
	}
	return setIntParam(e, p, valAsNum)
}
