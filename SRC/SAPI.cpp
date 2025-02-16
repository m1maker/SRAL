#ifdef _WIN32
#include "SAPI.h"
#include <cstdio>
#include<string>
#include<thread>

static WasapiPlayer* g_player = nullptr; // Make it global to avoid multiple voices when reinitializing


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

std::vector<PCMData> g_dataQueue;
bool g_threadStarted = false;
static void sapi_thread() {
	if (g_player == nullptr) {
		g_threadStarted = false;
	}
	HRESULT hr;
	while (g_threadStarted) {
		Sleep(1);
		for (uint64_t i = 0; i < g_dataQueue.size(); ++i) {

			hr = g_player->feed(g_dataQueue[i].data, g_dataQueue[i].size, nullptr);
			if (FAILED(hr))continue;
			hr = g_player->sync();
			if (FAILED(hr))continue;
		}
		if (g_dataQueue.size() > 0)		g_dataQueue.clear();
	}
	delete g_player;
	g_player = nullptr;
}
bool SAPI::Initialize() {
	if (g_player) {
		g_threadStarted = false;
		g_player->stop();
	}
	instance = new blastspeak;

	if (blastspeak_initialize(instance) == 0) {
		delete instance;
		instance = nullptr;
		return false;
	}
	CoInitialize(nullptr);
	wchar_t device[] = L"";
	WasapiPlayer::ChunkCompletedCallback callback = nullptr;

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = instance->channels;
	wfx.nSamplesPerSec = instance->sample_rate;
	wfx.wBitsPerSample = instance->bits_per_sample;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.cbSize = 0;
	g_player = new WasapiPlayer(device, wfx, callback);
	HRESULT hr = g_player->open();
	if (FAILED(hr)) {
		delete g_player;
		g_player = nullptr;
		return false;
	}
	g_threadStarted = true;
	std::thread t(sapi_thread);
	t.detach();
	return true;
}
bool SAPI::Uninitialize() {
	if (instance == nullptr || g_player == nullptr)return false;
	g_threadStarted = false; // SAPI thread will be stopped when all messages was spoken
	blastspeak_destroy(instance);
	delete instance;
	instance = nullptr;
	return true;
}
bool SAPI::GetActive() {
	return instance != nullptr && g_player != nullptr;
}
bool SAPI::Speak(const char* text, bool interrupt) {
	if (instance == nullptr || g_player == nullptr)
		return false;
	if (interrupt) {
		StopSpeech();

	}
	if (wfx.nChannels != instance->channels || wfx.nSamplesPerSec != instance->sample_rate || wfx.wBitsPerSample != instance->bits_per_sample) {

		this->Uninitialize();
		this->Initialize();
	}
	std::string text_str(text);
	unsigned long bytes;
	char* audio_ptr = blastspeak_speak_to_memory(instance, &bytes, text_str.c_str());
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
void* SAPI::SpeakToMemory(const char* text, uint64_t* buffer_size, int*channels, int* sample_rate, int* bits_per_sample) {
	if (instance == nullptr)return nullptr;
	std::string text_str(text);
	unsigned long bytes;
	char* audio_ptr = blastspeak_speak_to_memory(instance, &bytes, text_str.c_str());
	if (audio_ptr == nullptr)
		return nullptr;

	char* final = trim(audio_ptr, &bytes, &wfx, this->trimThreshold);
	*buffer_size = bytes;
	*channels = instance->channels;
	*sample_rate = instance->sample_rate;
	*bits_per_sample = instance->bits_per_sample;
	return final;
}

bool SAPI::SetParameter(int param, void* value) {
	if (instance == nullptr)
		return false;

	switch (param) {
	case SAPI_TRIM_THRESHOLD:
		this->trimThreshold = *static_cast<int*>(value);
		break;
	case SPEECH_RATE:
		blastspeak_set_voice_rate(instance, *static_cast<int*>(value));
		return true;
	case SPEECH_VOLUME:
		blastspeak_set_voice_volume(instance, *static_cast<int*>(value));
		return true;
	case VOICE_INDEX:
		blastspeak_set_voice(instance, *static_cast<int*>(value));
	default:
		return false;
	}
	return true;
}

void* SAPI::GetParameter(int param) {
	if (instance == nullptr)
		return nullptr;

	switch (param) {
	case SAPI_TRIM_THRESHOLD:
		return new long(this->trimThreshold);
	case SPEECH_RATE: {
		long* val = new long;
		blastspeak_get_voice_rate(instance, val);
		return static_cast<void*>(val);
	}
	case SPEECH_VOLUME: {
		long* val = new long;
		blastspeak_get_voice_volume(instance, val);
		return static_cast<void*>(val);
	}
	case VOICE_LIST:
		if (voices)
			delete[] voices;
		voices = new const char*[instance->voice_count];
		for (int i = 0; i < instance->voice_count; ++i) {
			const char* voice_desc = blastspeak_get_voice_description(instance, i);
			voices[i] = new char[strlen(voice_desc) + 1];
			strcpy(const_cast<char*>(voices[i]), voice_desc);
		}
		return voices;
	case VOICE_COUNT:
		return new int(instance->voice_count);
	default:
		return nullptr;
	}
	return nullptr;
}

bool SAPI::StopSpeech() {
	if (g_player == nullptr)return false;
	g_dataQueue.clear();
	g_player->stop();
	this->paused = false;
	return true;
}
bool SAPI::PauseSpeech() {
	paused = true;
	return SUCCEEDED(g_player->pause());
}
bool SAPI::ResumeSpeech() {
	paused = false;
	return SUCCEEDED(g_player->resume());
}
void SAPI::SetVolume(uint64_t value) {
	if (instance == nullptr)return;
	blastspeak_set_voice_volume(instance, value);
}
uint64_t SAPI::GetVolume() {
	if (instance == nullptr)return 0;
	long value;
	blastspeak_get_voice_volume(instance, &value);
	return value;
}
void SAPI::SetRate(uint64_t value) {
	if (instance == nullptr)return;
	blastspeak_set_voice_rate(instance, value);
}
uint64_t SAPI::GetRate() {
	if (instance == nullptr)return 0;
	long value;
	blastspeak_get_voice_rate(instance, &value);
	return value;
}
uint64_t SAPI::GetVoiceCount() {
	if (instance == nullptr)return 0;
	return instance->voice_count;
}
const char* SAPI::GetVoiceName(uint64_t index) {
	if (instance == nullptr)return nullptr;
	return blastspeak_get_voice_description(instance, index);
}
bool SAPI::SetVoice(uint64_t index) {
	if (instance == nullptr)return false;
	return blastspeak_set_voice(instance, index);
}



#endif