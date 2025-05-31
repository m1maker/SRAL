#ifdef _WIN32
#include "SAPI.h"
#include <cstdio>
#include<string>
#include<thread>

static std::shared_ptr<WasapiPlayer> g_player;


static char* trim(char* data, unsigned long* size, WAVEFORMATEX* wfx, int threshold) {
	int channels = wfx->nChannels;
	int bytesPerSample = wfx->wBitsPerSample / 8;
	int samplesPerFrame = channels * bytesPerSample;
	int numSamples = *size / samplesPerFrame;
	int startIndex = 0;
	int endIndex = numSamples - 1;

	for (int i = 0; i < numSamples; i++) {
		int maxAbsValue = 0;
		for (int j = 0; j < channels; j++) {
			int absValue = abs(static_cast<int>(data[i * samplesPerFrame + j]));
			if (absValue > maxAbsValue) {
				maxAbsValue = absValue;
			}
		}
		if (maxAbsValue >= threshold) {
			startIndex = i;
			break;
		}
	}

	for (int i = numSamples - 1; i >= 0; i--) {
		int maxAbsValue = 0;
		for (int j = 0; j < channels; j++) {
			int absValue = abs(static_cast<int>(data[i * samplesPerFrame + j]));
			if (absValue > maxAbsValue) {
				maxAbsValue = absValue;
			}
		}
		if (maxAbsValue >= threshold) {
			endIndex = i;
			break;
		}
	}

	int trimmedSize = (endIndex - startIndex + 1) * samplesPerFrame;
	char* trimmedData = new char[trimmedSize];
	memcpy(trimmedData, data + startIndex * samplesPerFrame, trimmedSize);
	*size = trimmedSize;
	return trimmedData;
}


struct PCMData {
	unsigned char* data;
	unsigned long size;
};

static std::vector<PCMData> g_dataQueue;
static bool g_threadStarted = false;
static void sapi_thread() {
	if (g_player == nullptr) {
		g_threadStarted = false;
	}
	HRESULT hr;
	while (g_threadStarted && g_player) {
		Sleep(1);
		for (const PCMData& data : g_dataQueue) {

			hr = g_player->feed(data.data, data.size, nullptr);
			if (FAILED(hr))continue;
			hr = g_player->sync();
			if (FAILED(hr))continue;
		}
		if (!g_dataQueue.empty())		g_dataQueue.clear();
	}
}

namespace Sral {
	bool Sapi::Initialize() {
		if (instance) {
			instance.reset();
		}
		this->voiceIndex = 0;
		if (g_player) {
			g_threadStarted = false;
			g_player->stop();
			g_player.reset();
		}
		if (speechThread.joinable()) {
			speechThread.join();
		}

		instance = std::make_unique<blastspeak>();

		if (blastspeak_initialize(&*instance) == 0) {
			instance.reset();
			return false;
		}
		CoInitialize(nullptr);

		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = instance->channels;
		wfx.nSamplesPerSec = instance->sample_rate;
		wfx.wBitsPerSample = instance->bits_per_sample;
		wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize = 0;
		g_player = std::make_shared<WasapiPlayer>((wchar_t*)L"", wfx, callback);
		HRESULT hr = g_player->open();
		if (FAILED(hr)) {
			g_player.reset();
			return false;
		}
		g_threadStarted = true;
		speechThread = std::thread(sapi_thread);
		speechThread.detach();
		return true;
	}

	bool Sapi::Uninitialize() {
		this->voiceIndex = 0;
		if (!instance || g_player == nullptr)return false;
		g_threadStarted = false; // SAPI thread will be stopped when all messages was spoken
		blastspeak_destroy(&*instance);
		instance.reset();
		if (speechThread.joinable()) {
			speechThread.join();
		}
		if (g_player) {
			g_player.reset();
		}
		return true;
	}

	bool Sapi::GetActive() {
		return instance && g_player != nullptr;
	}

	bool Sapi::Speak(const char* text, bool interrupt) {
		if (instance == nullptr || g_player == nullptr)
			return false;
		if (interrupt) {
			StopSpeech();

		}
		if (wfx.nChannels != instance->channels || wfx.nSamplesPerSec != instance->sample_rate || wfx.wBitsPerSample != instance->bits_per_sample) {

			wfx.nChannels = instance->channels;
			wfx.nSamplesPerSec = instance->sample_rate;
			wfx.wBitsPerSample = instance->bits_per_sample;
			wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
			wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
			g_threadStarted = false;
			if (speechThread.joinable()) {
				speechThread.join();
			}
			if (g_player) {
				g_player.reset();
			}


			g_player = std::make_shared<WasapiPlayer>((wchar_t*)L"", wfx, callback);
			HRESULT hr = g_player->open();
			if (FAILED(hr)) {
				g_player.reset();
				return false;
			}
			g_threadStarted = true;
			speechThread = std::thread(sapi_thread);
			speechThread.detach();
		}
		std::string text_str(text);
		unsigned long bytes;
		char* audio_ptr = blastspeak_speak_to_memory(&*instance, &bytes, text_str.c_str());
		if (audio_ptr == nullptr)
			return false;

		char* final = trim(audio_ptr, &bytes, &wfx, this->trimThreshold);
		if (final == nullptr)
			return false;
		PCMData dat = { 0, 0 };
		dat.data = (unsigned char*)final;
		dat.size = bytes;
		if (this->paused) {
			this->paused = false;
			if (!interrupt)
				g_player->resume();
		}
		g_dataQueue.push_back(dat);
		return true;
	}
	void* Sapi::SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample) {
		if (instance == nullptr)return nullptr;
		std::string text_str(text);
		unsigned long bytes;
		char* audio_ptr = blastspeak_speak_to_memory(&*instance, &bytes, text_str.c_str());
		if (audio_ptr == nullptr)
			return nullptr;

		char* final = trim(audio_ptr, &bytes, &wfx, this->trimThreshold);
		*buffer_size = bytes;
		*channels = instance->channels;
		*sample_rate = instance->sample_rate;
		*bits_per_sample = instance->bits_per_sample;
		return final;
	}

	bool Sapi::IsSpeaking() {
		return !paused && !g_dataQueue.empty();
	}

	bool Sapi::SetParameter(int param, const void* value) {
		if (instance == nullptr)
			return false;

		switch (param) {
		case SRAL_PARAM_SAPI_TRIM_THRESHOLD:
			this->trimThreshold = *reinterpret_cast<const int*>(value);
			break;
		case SRAL_PARAM_SPEECH_RATE:
			return blastspeak_set_voice_rate(&*instance, *reinterpret_cast<const long*>(value));
		case SRAL_PARAM_SPEECH_VOLUME:
			return blastspeak_set_voice_volume(&*instance, *reinterpret_cast<const long*>(value));
		case SRAL_PARAM_VOICE_INDEX: {
			int result = blastspeak_set_voice(&*instance, *reinterpret_cast<const int*>(value));
			if (result) {
				this->voiceIndex = *reinterpret_cast<const int*>(value);
				return true;
			}
			return false;
		}
		default:
			return false;
		}
		return true;
	}

	bool Sapi::GetParameter(int param, void* value) {
		if (instance == nullptr)
			return false;

		switch (param) {
		case SRAL_PARAM_SAPI_TRIM_THRESHOLD:
			*(int*)value = this->trimThreshold;
			return true;
		case SRAL_PARAM_SPEECH_RATE: {
			return blastspeak_get_voice_rate(&*instance, (long*)value);
		}
		case SRAL_PARAM_SPEECH_VOLUME: {
			return blastspeak_get_voice_volume(&*instance, (long*)value);
		}
		case SRAL_PARAM_VOICE_LIST: {
			char** voices = (char**)value;
			for (int i = 0; i < instance->voice_count; ++i) {
				const char* voice_desc = blastspeak_get_voice_description(&*instance, i);
				strcpy(voices[i], voice_desc);
			}
			return true;
		}
		case SRAL_PARAM_VOICE_COUNT:
			*(int*)value = instance->voice_count;
			return true;
		case SRAL_PARAM_VOICE_INDEX:
			*(int*)value = this->voiceIndex;
			return true;
		default:
			return false;
		}
		return false;
	}

	bool Sapi::StopSpeech() {
		if (g_player == nullptr)return false;
		g_dataQueue.clear();
		g_player->stop();
		this->paused = false;
		return true;
	}
	bool Sapi::PauseSpeech() {
		paused = true;
		return SUCCEEDED(g_player->pause());
	}
	bool Sapi::ResumeSpeech() {
		paused = false;
		return SUCCEEDED(g_player->resume());
	}


}
#endif