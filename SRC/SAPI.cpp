#ifdef _WIN32
#include "SAPI.h"
#define MA_NO_DSOUND
#define MA_NO_WINMM
#define MA_ENABLE_WASAPI
#define MA_NO_ENCODING
#define MA_NO_FLAC
#define MA_NO_WAV
#define MA_NO_MP3
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_GENERATION
#define MA_NO_ALSA
#define MA_NO_JACK
#define MA_NO_COREAUDIO
#define MA_NO_SNDIO
#define MA_NO_AUDIO4
#define MA_NO_OSS
#define MA_NO_AAUDIO
#define MA_NO_OPENSL
#define MA_NO_WEBAUDIO
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_NO_SSE2
#define MA_NO_AVX2
#define MA_NO_NEON
#define MINIAUDIO_IMPLEMENTATION
#include "../Dep/miniaudio.h"

#include <thread>
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


bool SAPI::Initialize() {
	m_audioEngine = (ma_engine*)new ma_engine;
	ma_result result = ma_engine_init(nullptr, (ma_engine*)m_audioEngine);
	if (result != MA_SUCCESS)return false;
	instance = new blastspeak;
	return blastspeak_initialize(instance) == 0;
}
bool SAPI::Uninitialize() {
	if (instance == nullptr)return false;
	blastspeak_destroy(instance);
	delete instance;
	instance = nullptr;
	ma_engine_uninit((ma_engine*)m_audioEngine);
	delete m_audioEngine;
	m_audioEngine = nullptr;
	return true;
}
bool SAPI::GetActive() {
	return instance != nullptr;
}
bool SAPI::Speak(const char* text, bool interrupt) {
	if (instance == nullptr)
		return false;

	unsigned long bytes;
	char* audio_ptr = blastspeak_speak_to_memory(instance, &bytes, text);
	if (audio_ptr == nullptr)
		return false;

	char* final = minitrim(audio_ptr, &bytes, instance->bits_per_sample, instance->channels);
	if (final == nullptr)
		return false;

	if (m_bufferInitialized) {
		ma_audio_buffer_uninit((ma_audio_buffer*)m_buffer);
		m_bufferInitialized = false;
	}

	ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(ma_format_s16, instance->channels, bytes / 2, (const void*)final, nullptr);
	bufferConfig.sampleRate = 16000;
	bufferConfig.channels = 1;
	m_buffer = (ma_audio_buffer*) new ma_audio_buffer;
	ma_result result = ma_audio_buffer_init(&bufferConfig, (ma_audio_buffer*)m_buffer);
	if (result != MA_SUCCESS)
		return false;
	m_bufferInitialized = true;

	std::unique_ptr<std::thread> t(new std::thread([this, interrupt]() {
		if (!interrupt) {
			if (m_soundInitialized) {
				while (ma_sound_is_playing((ma_sound*)m_sound) == MA_TRUE) {
				}
				ma_sound_uninit((ma_sound*)m_sound);
				m_soundInitialized = false;
			}
		}
		else {
			if (m_soundInitialized) {
				ma_sound_uninit((ma_sound*)m_sound);
				m_soundInitialized = false;
			}

		}
		m_sound = (ma_sound*) new ma_sound;
		ma_result res = ma_sound_init_from_data_source((ma_engine*)m_audioEngine, (ma_audio_buffer*)m_buffer, 0, nullptr, (ma_sound*)m_sound);
		if (res != MA_SUCCESS)
			return;
		m_soundInitialized = true;
		ma_sound_start((ma_sound*)m_sound);
		}));

	t->join();
	return true;
}

bool SAPI::StopSpeech() {
	if (m_soundInitialized)
		return ma_sound_stop((ma_sound*)m_sound) == MA_SUCCESS;
	return false;
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
#endif