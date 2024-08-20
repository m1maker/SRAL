#define NOMINMAX
#include "wasapi.h"
#include <algorithm>
#include <atlbase.h>
#include <atlcomcli.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <string>
#include <vector>
#include <windows.h>

constexpr REFERENCE_TIME REFTIMES_PER_MILLISEC = 10000;
constexpr REFERENCE_TIME BUFFER_SIZE = 400 * REFTIMES_PER_MILLISEC;

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioClock = __uuidof(IAudioClock);
const IID IID_IMMNotificationClient = __uuidof(IMMNotificationClient);
const IID IID_IAudioStreamVolume = __uuidof(IAudioStreamVolume);
CComPtr<NotificationClient> notificationClient;

WasapiPlayer::WasapiPlayer(wchar_t* deviceName, WAVEFORMATEX format, ChunkCompletedCallback callback)
	: deviceName(deviceName), format(format), callback(callback) {
	wakeEvent = CreateEvent(nullptr, false, false, nullptr);
	CComPtr<IMMDeviceEnumerator> enumerator;
	HRESULT hr = enumerator.CoCreateInstance(CLSID_MMDeviceEnumerator);
	notificationClient = new NotificationClient();
	enumerator->RegisterEndpointNotificationCallback(notificationClient);

}

HRESULT WasapiPlayer::open(bool force) {
	if (client && !force) {
		return S_OK;
	}
	defaultDeviceChangeCount = notificationClient->getDefaultDeviceChangeCount();
	deviceStateChangeCount = notificationClient->getDeviceStateChangeCount();
	CComPtr<IMMDeviceEnumerator> enumerator;
	HRESULT hr = enumerator.CoCreateInstance(CLSID_MMDeviceEnumerator);
	if (FAILED(hr)) {
		return hr;
	}
	CComPtr<IMMDevice> device;
	isUsingPreferredDevice = false;
	if (deviceName.empty()) {
		hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
	}
	else {
		hr = getPreferredDevice(device);
		if (SUCCEEDED(hr)) {
			isUsingPreferredDevice = true;
		}
		else {
			hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
		}
	}
	if (FAILED(hr)) {
		return hr;
	}
	hr = device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&client);
	if (FAILED(hr)) {
		return hr;
	}
	hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
		BUFFER_SIZE, 0, &format, nullptr);
	if (FAILED(hr)) {
		return hr;
	}
	hr = client->GetBufferSize(&bufferFrames);
	if (FAILED(hr)) {
		return hr;
	}
	hr = client->GetService(IID_IAudioRenderClient, (void**)&render);
	if (FAILED(hr)) {
		return hr;
	}
	hr = client->GetService(IID_IAudioClock, (void**)&clock);
	if (FAILED(hr)) {
		return hr;
	}
	hr = clock->GetFrequency(&clockFreq);
	if (FAILED(hr)) {
		return hr;
	}
	playState = PlayState::stopped;
	return S_OK;
}

HRESULT WasapiPlayer::feed(unsigned char* data, unsigned int size, unsigned int* id
) {
	if (playState == PlayState::stopping) {
		completeStop();
	}
	UINT32 remainingFrames = size / format.nBlockAlign;
	HRESULT hr;

	auto reopenUsingNewDev = [&] {
		hr = open(true);
		if (FAILED(hr)) {
			return false;
		}
		for (auto& [itemId, itemEnd] : feedEnds) {
			callback(this, itemId);
		}
		feedEnds.clear();
		sentFrames = 0;
		return true;
		};

	while (remainingFrames > 0) {
		UINT32 paddingFrames;

		auto getPaddingHandlingStopOrDevChange = [&] {
			if (playState == PlayState::stopping) {
				completeStop();
				hr = S_OK;
				return false;
			}
			if (
				didPreferredDeviceBecomeAvailable() ||
				(!isUsingPreferredDevice && defaultDeviceChangeCount !=
					notificationClient->getDefaultDeviceChangeCount())
				) {
				if (!reopenUsingNewDev()) {
					return false;
				}
			}
			hr = client->GetCurrentPadding(&paddingFrames);
			if (
				hr == AUDCLNT_E_DEVICE_INVALIDATED
				|| hr == AUDCLNT_E_NOT_INITIALIZED
				) {
				if (!reopenUsingNewDev()) {
					return false;
				}
				hr = client->GetCurrentPadding(&paddingFrames);
			}
			return SUCCEEDED(hr);
			};

		if (!getPaddingHandlingStopOrDevChange()) {
			return hr;
		}
		if (paddingFrames > bufferFrames / 2) {
			waitUntilNeeded(framesToMs(paddingFrames - bufferFrames / 2));
			if (!getPaddingHandlingStopOrDevChange()) {
				return hr;
			}
		}
		const UINT32 sendFrames = std::min(remainingFrames, bufferFrames - paddingFrames);
		const UINT32 sendBytes = sendFrames * format.nBlockAlign;
		BYTE* buffer;
		hr = render->GetBuffer(sendFrames, &buffer);
		if (FAILED(hr)) {
			return hr;
		}
		memcpy(buffer, data, sendBytes);
		hr = render->ReleaseBuffer(sendFrames, 0);
		if (FAILED(hr)) {
			return hr;
		}
		if (playState == PlayState::stopped) {
			hr = client->Start();
			if (FAILED(hr)) {
				return hr;
			}
			if (playState == PlayState::stopping) {
				completeStop();
				return S_OK;
			}
			playState = PlayState::playing;
		}
		maybeFireCallback();
		data += sendBytes;
		size -= sendBytes;
		remainingFrames -= sendFrames;
		sentFrames += sendFrames;
	}

	if (playState == PlayState::playing) {
		maybeFireCallback();
	}
	if (id) {
		*id = nextFeedId++;
		feedEnds.push_back({ *id, framesToMs(sentFrames) });
	}
	return S_OK;
}

void WasapiPlayer::maybeFireCallback() {
	const UINT64 playPos = getPlayPos();
	std::erase_if(feedEnds, [&](auto& val) {
		auto [id, end] = val;
		if (playPos >= end) {
			callback(this, id);
			return true;
		}
		return false;
		});
}

UINT64 WasapiPlayer::getPlayPos() {
	UINT64 pos;
	HRESULT hr = clock->GetPosition(&pos, nullptr);
	if (FAILED(hr)) {
		return framesToMs(sentFrames);
	}
	return pos * 1000 / clockFreq;
}

void WasapiPlayer::waitUntilNeeded(UINT64 maxWait) {
	if (!feedEnds.empty()) {
		UINT64 feedEnd = feedEnds[0].second;
		const UINT64 nextCallbackTime = feedEnd - getPlayPos();
		if (nextCallbackTime < maxWait) {
			maxWait = nextCallbackTime;
		}
	}
	WaitForSingleObject(wakeEvent, (DWORD)maxWait);
}

HRESULT WasapiPlayer::getPreferredDevice(CComPtr<IMMDevice>& preferredDevice) {
	CComPtr<IMMDeviceEnumerator> enumerator;
	HRESULT hr = enumerator.CoCreateInstance(CLSID_MMDeviceEnumerator);
	if (FAILED(hr)) {
		return hr;
	}
	CComPtr<IMMDeviceCollection> devices;
	hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);
	if (FAILED(hr)) {
		return hr;
	}
	UINT count = 0;
	devices->GetCount(&count);
	for (UINT d = 0; d < count; ++d) {
		CComPtr<IMMDevice> device;
		hr = devices->Item(d, &device);
		if (FAILED(hr)) {
			return hr;
		}
		CComPtr<IPropertyStore> props;
		hr = device->OpenPropertyStore(STGM_READ, &props);
		if (FAILED(hr)) {
			return hr;
		}
		PROPVARIANT val;
		hr = props->GetValue(PKEY_Device_FriendlyName, &val);
		if (FAILED(hr)) {
			return hr;
		}
		constexpr size_t MAX_CHARS = MAXPNAMELEN - 1;
		if (wcsncmp(val.pwszVal, deviceName.c_str(), MAX_CHARS) == 0) {
			PropVariantClear(&val);
			preferredDevice = std::move(device);
			return S_OK;
		}
		PropVariantClear(&val);
	}
	return E_NOTFOUND;
}

bool WasapiPlayer::didPreferredDeviceBecomeAvailable() {
	if (
		isUsingPreferredDevice ||
		deviceName.empty() ||
		deviceStateChangeCount == notificationClient->getDeviceStateChangeCount()
		) {
		return false;
	}
	CComPtr<IMMDevice> device;
	return SUCCEEDED(getPreferredDevice(device));
}

HRESULT WasapiPlayer::stop() {
	playState = PlayState::stopping;
	HRESULT hr = client->Stop();
	if (
		hr != AUDCLNT_E_DEVICE_INVALIDATED
		&& hr != AUDCLNT_E_NOT_INITIALIZED
		) {
		if (FAILED(hr)) {
			return hr;
		}
		hr = client->Reset();
		if (FAILED(hr)) {
			return hr;
		}
	}
	SetEvent(wakeEvent);
	return S_OK;
}

void WasapiPlayer::completeStop() {
	nextFeedId = 0;
	sentFrames = 0;
	feedEnds.clear();
	playState = PlayState::stopped;
}

HRESULT WasapiPlayer::sync() {
	UINT64 sentMs = framesToMs(sentFrames);
	for (UINT64 playPos = getPlayPos(); playPos < sentMs;
		playPos = getPlayPos()) {
		if (playState != PlayState::playing) {
			return S_OK;
		}
		maybeFireCallback();
		waitUntilNeeded(sentMs - playPos);
	}
	if (playState == PlayState::playing) {
		maybeFireCallback();
	}
	return S_OK;
}

HRESULT WasapiPlayer::idle() {
	HRESULT hr = sync();
	if (FAILED(hr)) {
		return hr;
	}
	hr = stop();
	if (FAILED(hr)) {
		return hr;
	}
	completeStop();
	return S_OK;
}

HRESULT WasapiPlayer::pause() {
	if (playState != PlayState::playing) {
		return S_OK;
	}
	HRESULT hr = client->Stop();
	if (FAILED(hr)) {
		return hr;
	}
	return S_OK;
}

HRESULT WasapiPlayer::resume() {
	if (playState != PlayState::playing) {
		return S_OK;
	}
	HRESULT hr = client->Start();
	if (FAILED(hr)) {
		return hr;
	}
	return S_OK;
}

HRESULT WasapiPlayer::setChannelVolume(unsigned int channel, float level) {
	CComPtr<IAudioStreamVolume> volume;
	HRESULT hr = client->GetService(IID_IAudioStreamVolume, (void**)&volume);
	if (hr == AUDCLNT_E_DEVICE_INVALIDATED) {
		hr = open(true);
		if (FAILED(hr)) {
			return hr;
		}
		hr = client->GetService(IID_IAudioStreamVolume, (void**)&volume);
	}
	if (FAILED(hr)) {
		return hr;
	}
	return volume->SetChannelVolume(channel, level);
}

