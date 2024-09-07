#ifdef _WIN32
#include "SAPI.h"
#include "Util.h"
#include <cstdio>
#include<string>
#include<thread>


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
static void sapi_thread(WasapiPlayer* player) {
	if (player == nullptr) {
		g_threadStarted = false;
	}
	HRESULT hr;
	while (g_threadStarted) {
		Sleep(1);
		for (uint64_t i = 0; i < g_dataQueue.size(); ++i) {

			hr = player->feed(g_dataQueue[i].data, g_dataQueue[i].size, nullptr);
			if (FAILED(hr))continue;
			hr = player->sync();
			if (FAILED(hr))continue;
		}
		if (g_dataQueue.size() > 0)		g_dataQueue.clear();
	}
}
bool SAPI::Initialize() {
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
	player = new WasapiPlayer(device, wfx, callback);
	HRESULT hr = player->open();
	if (FAILED(hr)) {
		delete player;
		player = nullptr;
		return false;
	}
	g_threadStarted = true;
	std::thread t(sapi_thread, player);
	t.detach();
	return true;
}
bool SAPI::Uninitialize() {
	if (instance == nullptr || player == nullptr)return false;
	g_threadStarted = false;
	blastspeak_destroy(instance);
	delete instance;
	instance = nullptr;
	StopSpeech();
	delete player;
	player = nullptr;
	return true;
}
bool SAPI::GetActive() {
	return instance != nullptr;
}
bool SAPI::Speak(const char* text, bool interrupt) {
	if (instance == nullptr || player == nullptr)
		return false;
	if (interrupt) {
		StopSpeech();

	}
	if (wfx.nChannels != instance->channels || wfx.nSamplesPerSec != instance->sample_rate || wfx.wBitsPerSample != instance->bits_per_sample) {

		this->Uninitialize();
		this->Initialize();
	}
	std::string text_str(text);
	if (!IsSsml(text_str))AddSsml(text_str);
	unsigned long bytes;
	char* audio_ptr = blastspeak_speak_to_memory(instance, &bytes, text_str.c_str());
	if (audio_ptr == nullptr)
		return false;

	char* final = trim(audio_ptr, &bytes, &wfx, 20);
	if (final == nullptr)
		return false;
	PCMData dat = { 0, 0 };
	dat.data = (unsigned char*)final;
	dat.size = bytes;
	if (this->paused) {
		this->paused = false;
		if (!interrupt)
			player->resume();
	}
	g_dataQueue.push_back(dat);
	return true;
}

bool SAPI::StopSpeech() {
	if (player == nullptr)return false;
	g_dataQueue.clear();
	player->stop();
	this->paused = false;
	return true;
}
bool SAPI::PauseSpeech() {
	paused = true;
	return SUCCEEDED(player->pause());
}
bool SAPI::ResumeSpeech() {
	paused = false;
	return SUCCEEDED(player->resume());
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