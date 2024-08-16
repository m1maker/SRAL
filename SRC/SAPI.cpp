#ifdef _WIN32
#include "SAPI.h"
#define MA_NO_DSOUND
#define MA_NO_WINMM
#define MA_ENABLE_WASAPI
#define MA_NO_ENCODING
#define MA_NO_FLAC
#define MA_NO_WAV
#define MA_NO_MP3
#define MA_NO_ENGINE
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
#include <vector>
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
std::vector<ma_audio_buffer> g_buffers;
void device_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	for (uint64_t i = 0; i < g_buffers.size(); ++i) {
		ma_uint64 length;
		ma_uint64 cursor;
		ma_audio_buffer_get_length_in_pcm_frames(&g_buffers[i], &length);
		ma_audio_buffer_get_cursor_in_pcm_frames(&g_buffers[i], &cursor);
		if (cursor < length) {
			ma_audio_buffer_read_pcm_frames(&g_buffers[i], pOutput, frameCount, MA_FALSE);
			ma_audio_buffer_get_cursor_in_pcm_frames(&g_buffers[i], &cursor);
		}
		else {
			ma_audio_buffer_uninit(&g_buffers[i]);
			g_buffers.erase(g_buffers.begin() + i);
		}
	}
	(void)pOutput;
}


bool SAPI::Initialize() {
	ma_device_config conf = ma_device_config_init(ma_device_type_playback);
	conf.playback.channels = 1;
	conf.sampleRate = 16000;
	conf.playback.format = ma_format_s16;
	conf.dataCallback = device_callback;
	m_audioDevice = (ma_device*) new ma_device;
	ma_result res = ma_device_init(nullptr, &conf, (ma_device*)m_audioDevice);
	if (res != MA_SUCCESS)return false;
	m_deviceInitialized = true;
	instance = new blastspeak;
	return blastspeak_initialize(instance) == 0;
}
bool SAPI::Uninitialize() {
	if (instance == nullptr)return false;
	blastspeak_destroy(instance);
	delete instance;
	instance = nullptr;
	if (m_audioDevice == nullptr)return false;
	ma_device_uninit((ma_device*)m_audioDevice);
	delete m_audioDevice;
	m_audioDevice = nullptr;
	m_deviceInitialized = false;
	return true;
}
bool SAPI::GetActive() {
	return instance != nullptr;
}
bool SAPI::Speak(const char* text, bool interrupt) {
	if (instance == nullptr)
		return false;
	if (interrupt)
		StopSpeech();
	unsigned long bytes;
	char* audio_ptr = blastspeak_speak_to_memory(instance, &bytes, text);
	if (audio_ptr == nullptr)
		return false;

	char* final = minitrim(audio_ptr, &bytes, instance->bits_per_sample, instance->channels);
	if (final == nullptr)
		return false;
	ma_audio_buffer buffer;
	ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(ma_format_s16, instance->channels, bytes / 2, (const void*)final, nullptr);
	bufferConfig.sampleRate = instance->sample_rate;
	bufferConfig.channels = instance->channels;
	ma_result result = ma_audio_buffer_init(&bufferConfig, &buffer);
	if (result != MA_SUCCESS)
		return false;
	if (interrupt || ma_device_is_started((ma_device*)m_audioDevice) == MA_FALSE)ma_device_start((ma_device*)m_audioDevice);
	g_buffers.push_back(buffer);
	return true;
}

bool SAPI::StopSpeech() {
	if (!m_deviceInitialized)return false;
	ma_device_stop((ma_device*)m_audioDevice);
	for (uint64_t i = 0; i < g_buffers.size(); ++i) {
		ma_audio_buffer_uninit(&g_buffers[i]);
	}
	g_buffers.clear();
	return true;
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