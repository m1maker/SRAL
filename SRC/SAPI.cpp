#ifdef _WIN32
#include "SAPI.h"
#include <cstdio>
#include<string>
// This function is taken from [NVGT](https://github.com/samtupy/nvgt)
static char* minitrim(char* data, unsigned long* bufsize, int bitrate, int channels) {
	char* ptr = data;
	if (!ptr || !bufsize || *bufsize % 2 != 0 || *bufsize < 1) return ptr;
	short a = 3072;
	while (bitrate == 16 && (ptr - data) < *bufsize) {
		if (channels == 2) {
			short l = (((short)*ptr) << 8) | *(ptr + 1);
			short r = (((short)*(ptr + 2)) << 8) | *(ptr + 3);
			if (l > -a && l < a && r > -a && r < a)
				ptr += 4;
			else break;
		}
		else if (channels == 1) {
			short s = (((short)*ptr) << 8) | *(ptr + 1);
			if (s > -a && s < a)
				ptr += 2;
			else break;
		}
	}
	*bufsize -= (ptr - data);
	return ptr;
}
/*
Attention!
Here I'm using WaveOut, which is not the best solution today.
I haven't been able to find a solution on how to quickly and fail-free playback PCM data via Wasapi.
Please, if you have a solution, you can contribute to the project. In any case, I will try to fix it.
*/



bool SAPI::Initialize() {
	instance = new blastspeak;
	blastspeak_initialize(instance);
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = instance->channels;
	wfx.nSamplesPerSec = instance->sample_rate;
	wfx.wBitsPerSample = instance->bits_per_sample;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.cbSize = 0;
	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, WAVE_FORMAT_DIRECT) != MMSYSERR_NOERROR) {
		return false;
	}
	return true;
}
bool SAPI::Uninitialize() {
	if (instance == nullptr)return false;
	blastspeak_destroy(instance);
	delete instance;
	instance = nullptr;
	StopSpeech();
	waveOutClose(hWaveOut);
	return true;
}
bool SAPI::GetActive() {
	return instance != nullptr;
}
bool SAPI::Speak(const char* text, bool interrupt) {
	if (instance == nullptr)
		return false;
	if (interrupt) {
		StopSpeech();
		waveOutRestart(hWaveOut);

	}
	unsigned long bytes;
	char* audio_ptr = blastspeak_speak_to_memory(instance, &bytes, text);
	if (audio_ptr == nullptr)
		return false;

	char* final = minitrim(audio_ptr, &bytes, instance->bits_per_sample, instance->channels);
	if (final == nullptr)
		return false;
	memset(&wh, 0, sizeof(WAVEHDR));
	wh.lpData = final;
	wh.dwBufferLength = bytes;
	wh.dwFlags = 0;

	if (waveOutPrepareHeader(hWaveOut, &wh, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
		return false;
	}


	if (waveOutWrite(hWaveOut, &wh, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
		waveOutUnprepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
		return false;
	}


	return true;
}

bool SAPI::StopSpeech() {
	waveOutReset(hWaveOut);

	waveOutUnprepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
	return true;
}
void SAPI::SetVolume(uint64_t value) {
	if (instance == nullptr)return;
	this->Uninitialize();
	this->Initialize();
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
	this->Uninitialize();
	this->Initialize();
	blastspeak_set_voice_rate(instance, value);
}
uint64_t SAPI::GetRate() {
	if (instance == nullptr)return 0;
	long value;
	blastspeak_get_voice_rate(instance, &value);
	return value;
}
#endif