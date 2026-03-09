#pragma once
#include "../Include/SRAL.h"
#include "Engine.h"

class NSSpeechSynthesizerWrapper;

namespace Sral {
    class NsSpeech final : public Engine {
    public:
			bool Initialize() override;
        bool Uninitialize() override;
        bool Speak(const char* text, bool interrupt) override;
        bool StopSpeech() override;
        bool IsSpeaking() override;
        bool GetActive() override;
        bool SetParameter(int param, const void* value) override;
        bool GetParameter(int param, void* value) override;
        
        int GetNumber() override { return SRAL_ENGINE_NS_SPEECH; }
        int GetFeatures() override { 
            return SRAL_SUPPORTS_SPEECH | SRAL_SUPPORTS_SPEECH_RATE | SRAL_SUPPORTS_SPEECH_VOLUME; 
        }

    private:
        NSSpeechSynthesizerWrapper* obj = nullptr;
    };
}
