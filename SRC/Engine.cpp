#include "../Include/SRAL.h"
#include "Engine.h"
#include <cstddef>

namespace Sral {
	Engine::Engine() {

	}
	Engine::~Engine() {
		ReleaseAllStrings();
		Uninitialize();
	}

	bool Engine::Braille(const char* text) {
		(void)text;
		return false;
	}

	bool Engine::GetActive() {
		return false;
	}

	int Engine::GetFeatures() {
		return 0;
	}

	int Engine::GetKeyFlags() {
		return HANDLE_NONE;
	}

	int Engine::GetNumber() {
		return SRAL_ENGINE_NONE;
	}

	bool Engine::GetParameter(int parameter, void* value) {
		(void)parameter;
		(void)value;
		return false;
	}

	bool Engine::Initialize() {
		return false;
	}

	bool Engine::IsSpeaking() {
		return false;
	}

	bool Engine::PauseSpeech() {
		return false;
	}

	bool Engine::ResumeSpeech() {
		return false;
	}

	bool Engine::SetParameter(int parameter, const void* value) {
		(void)parameter;
		(void)value;
		return false;
	}

	bool Engine::Speak(const char* text, bool interrupt) {
		(void)text;
		(void)interrupt;
		return false;
	}

	bool Engine::SpeakSsml(const char* ssml, bool interrupt) {
		(void)ssml;
		(void)interrupt;
		return false;
	}

	void* Engine::SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample) {
		(void)text;
		(void)buffer_size;
		(void)channels;
		(void)sample_rate;
		(void)bits_per_sample;
		return nullptr;
	}

	bool Engine::StopSpeech() {
		return false;
	}

	bool Engine::Uninitialize() {
		return false;
	}
}
