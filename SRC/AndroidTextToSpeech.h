#ifndef SAPI_H_
#define SAPI_H_
#pragma once
#include "../Include/SRAL.h"
#include "Engine.h"
#include <memory>
#include <thread>
#include <jni.h>

namespace Sral {
	class AndroidTextToSpeech final : public Engine {
	public:
		bool Speak(const char* text, bool interrupt)override;
		bool SpeakSsml(const char* ssml, bool interrupt)override {
			return false;
		}
		void* SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample)override { return false;}
		bool SetParameter(int param, const void* value)override;
		bool GetParameter(int param, void* value) override;

		bool Braille(const char* text)override { return false; }
		bool StopSpeech()override;
		bool PauseSpeech()override {return false;}
		bool ResumeSpeech()override {return false;}
		bool IsSpeaking() override;
		int GetNumber()override {
			return SRAL_ENGINE_ANDROID_TEXT_TO_SPEECH;
		}
		bool GetActive()override;
		bool Initialize()override;
		bool Uninitialize()override;
		int GetFeatures()override {
			return SRAL_SUPPORTS_SPEECH | SRAL_SUPPORTS_SPEECH_RATE | SRAL_SUPPORTS_SPEECH_VOLUME | SRAL_SUPPORTS_SELECT_VOICE;
		}
		int GetKeyFlags()override {
			return HANDLE_NONE;
		}

	private:
		jclass speechClass;
		jmethodID constructor, midIsActive, midIsSpeaking, midSpeak, midSilence, midGetVoice, midSetRate, midSetPitch, midSetVolume, midGetVoices, midSetVoice, midGetMaxSpeechInputLength, midGetPitch, midGetRate, midGetVolume;
		JNIEnv *env;
		jobject speechObj;
	};
}
#endif