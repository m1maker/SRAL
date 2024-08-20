#pragma once

#ifndef WASAPI_HPP
#define WASAPI_HPP

#if defined(_WIN32)
#ifdef WASAPI_EXPORTS
#define WASAPI_API __declspec(dllexport)
#else
#define WASAPI_API __declspec(dllimport)
#endif
#else
#define WASAPI_API
#endif

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

class AutoHandle {
public:
	AutoHandle() : handle(nullptr) {}
	AutoHandle(HANDLE handle) : handle(handle) {}

	~AutoHandle() {
		if (handle) {
			CloseHandle(handle);
		}
	}

	AutoHandle& operator=(HANDLE newHandle) {
		if (handle) {
			CloseHandle(handle);
		}
		handle = newHandle;
		return *this;
	}

	operator HANDLE() {
		return handle;
	}

private:
	HANDLE handle;
};

class NotificationClient : public IMMNotificationClient {
public:
	ULONG STDMETHODCALLTYPE AddRef() override {
		return InterlockedIncrement(&refCount);
	}

	ULONG STDMETHODCALLTYPE Release() override {
		LONG result = InterlockedDecrement(&refCount);
		if (result == 0) {
			delete this;
		}
		return result;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) final {
		if (riid == IID_IUnknown || riid == IID_IMMNotificationClient) {
			AddRef();
			*ppvObject = (void*)this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP OnDefaultDeviceChanged(EDataFlow flow, ERole     role,
		LPCWSTR   defaultDeviceId
	) final {
		if (flow == eRender && role == eConsole) {
			++defaultDeviceChangeCount;
		}
		return S_OK;
	}

	STDMETHODIMP OnDeviceAdded(LPCWSTR deviceId) final {
		return S_OK;
	}

	STDMETHODIMP OnDeviceRemoved(LPCWSTR deviceId) final {
		return S_OK;
	}

	STDMETHODIMP OnDeviceStateChanged(LPCWSTR deviceId, DWORD   newState) final {
		++deviceStateChangeCount;
		return S_OK;
	}

	STDMETHODIMP OnPropertyValueChanged(LPCWSTR           deviceId,
		const PROPERTYKEY key
	) final {
		return S_OK;
	}

	unsigned int getDefaultDeviceChangeCount() {
		return defaultDeviceChangeCount;
	}

	unsigned int getDeviceStateChangeCount() {
		return deviceStateChangeCount;
	}

private:
	LONG refCount = 0;
	unsigned int defaultDeviceChangeCount = 0;
	unsigned int deviceStateChangeCount = 0;
};


class WasapiPlayer {
public:
	using ChunkCompletedCallback = void(*)(WasapiPlayer* player, unsigned int id);

	WasapiPlayer(wchar_t* deviceName, WAVEFORMATEX format, ChunkCompletedCallback callback);
	HRESULT open(bool force = false);
	HRESULT feed(unsigned char* data, unsigned int size, unsigned int* id);
	HRESULT stop();
	HRESULT sync();
	HRESULT idle();
	HRESULT pause();
	HRESULT resume();
	HRESULT setChannelVolume(unsigned int channel, float level);

private:
	void maybeFireCallback();
	void completeStop();

	UINT64 framesToMs(UINT32 frames) {
		return frames * 1000 / format.nSamplesPerSec;
	}

	UINT64 getPlayPos();
	void waitUntilNeeded(UINT64 maxWait = INFINITE);
	HRESULT getPreferredDevice(CComPtr<IMMDevice>& preferredDevice);
	bool didPreferredDeviceBecomeAvailable();

	enum class PlayState {
		stopped,
		playing,
		stopping,
	};

	CComPtr<IAudioClient> client;
	CComPtr<IAudioRenderClient> render;
	CComPtr<IAudioClock> clock;
	UINT32 bufferFrames;
	std::wstring deviceName;
	WAVEFORMATEX format;
	ChunkCompletedCallback callback;
	PlayState playState = PlayState::stopped;
	std::vector<std::pair<unsigned int, UINT64>> feedEnds;
	UINT64 clockFreq;
	UINT32 sentFrames = 0;
	unsigned int nextFeedId = 0;
	AutoHandle wakeEvent;
	unsigned int defaultDeviceChangeCount;
	unsigned int deviceStateChangeCount;
	bool isUsingPreferredDevice = false;
};

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif
#endif
