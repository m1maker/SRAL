#ifdef _WIN32
#include "SAPI.h"
#include <cstdio>
#include<string>
#include<thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <functional>

static std::shared_ptr<WasapiPlayer> g_player;


template<typename T, typename Func, typename ...Args>
inline void safeCall(T* obj, Func func, Args... args) {
	if (obj) {
		(obj->*func)(args...);
	}
	else {
	}
}

template<typename T, typename Func, typename ...Args>
inline void safeCall(T* obj, Func func, Args... args, std::function<void()> onNull) {
	if (obj) {
		(obj->*func)(args...);
	}
	else {
		onNull();
	}
}

template<typename T, typename R, typename Func, typename ...Args>
inline std::optional<R> safeCallVal(T* obj, Func func, Args ...args) {
	if (obj) {
		return (obj->*func)(args...);
	}
	else {
		return std::nullopt;
	}
}



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
static std::mutex g_dataQueueMutex;
static std::condition_variable g_dataQueueCv;
static std::atomic<bool> g_threadStarted = false;

static void sapi_thread() {
	if (g_player == nullptr) {
		g_threadStarted.store(false);
	}
	while (g_threadStarted.load()) {
		PCMData current_data;
		{
			std::unique_lock<std::mutex> lock(g_dataQueueMutex);
			g_dataQueueCv.wait(lock, [&] { return !g_dataQueue.empty() || !g_threadStarted.load(); });

			if (!g_threadStarted.load() && g_dataQueue.empty()) {
				break;
			}
			if (g_dataQueue.empty()) {
				continue;
			}

			current_data = g_dataQueue.front();
			g_dataQueue.erase(g_dataQueue.begin());
		}

		if (current_data.data) {
			auto result = safeCallVal<WasapiPlayer, HRESULT>(g_player.get(), &WasapiPlayer::feed, current_data.data, current_data.size, nullptr);
			delete[] current_data.data;

			if (result.has_value() && SUCCEEDED(*result)) {
				result = safeCallVal<WasapiPlayer, HRESULT>(g_player.get(), &WasapiPlayer::sync);
			}
		}
	}
	std::unique_lock<std::mutex> lock(g_dataQueueMutex);
	for (PCMData& data : g_dataQueue) {
		if (data.data) {
			delete[] data.data;
			data.data = nullptr;
		}
	}
	g_dataQueue.clear();
}

namespace Sral {
	bool Sapi::Initialize() {
		if (instance) {
			instance.reset();
		}
		this->voiceIndex = 0;
		if (g_player) {
			g_threadStarted.store(false);
			g_player->stop();
			g_player.reset();
		}
		if (speechThread.joinable()) {
			g_dataQueueCv.notify_one();
			speechThread.join();
		}

		instance = std::make_unique<blastspeak>();

		if (blastspeak_initialize(&*instance) == 0) {
			instance.reset();
			return false;
		}

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
		g_threadStarted.store(true);
		speechThread = std::thread(sapi_thread);
		speechThread.detach();
		return true;
	}

	bool Sapi::Uninitialize() {
		ReleaseAllStrings();
		this->voiceIndex = 0;
		if (!instance || g_player == nullptr)return false;
		g_threadStarted.store(false);
		blastspeak_destroy(&*instance);
		instance.reset();
		if (speechThread.joinable()) {
			g_dataQueueCv.notify_one();
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
				g_dataQueueCv.notify_one();
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
		{
			std::unique_lock<std::mutex> lock(g_dataQueueMutex);
			g_dataQueue.push_back(dat);
		}
		g_dataQueueCv.notify_one();
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
		case SRAL_PARAM_VOICE_PROPERTIES: {
			ReleaseAllStrings();
			SRAL_VoiceInfo* voiceProperties = (SRAL_VoiceInfo*)value;
			int index = 0;
			for (index; voiceProperties && instance && index < instance->voice_count; ++index) {
				voiceProperties[index].index = index;
				voiceProperties[index].name = AddString(blastspeak_get_voice_description(&*instance, index));
				voiceProperties[index].language = AddString(blastspeak_get_voice_languages(&*instance, index));
				voiceProperties[index].gender = AddString(blastspeak_get_voice_attribute(&*instance, index, "Gender"));
				voiceProperties[index].vendor = AddString(blastspeak_get_voice_attribute(&*instance, index, "Vendor"));
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
		if (g_player == nullptr) return false;
		{
			std::unique_lock<std::mutex> lock(g_dataQueueMutex);
			for (PCMData& data : g_dataQueue) {
				if (data.data) {
					delete[] data.data;
					data.data = nullptr;
				}
			}
			g_dataQueue.clear();
		}
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