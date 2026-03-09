#include "NsSpeech.h"
#import <AppKit/AppKit.h>

class NSSpeechSynthesizerWrapper {
public:
    NSSpeechSynthesizer* synth;
    float rate;
    float volume;

    NSSpeechSynthesizerWrapper() : synth(nullptr), rate(175.0f), volume(1.0f) {}

    bool Initialize() {
        synth = [[NSSpeechSynthesizer alloc] init];
        if (synth) {
            rate = [synth rate];
            volume = [synth volume];
        }
        return synth != nil;
    }

    void Uninitialize() {
        if (synth) {
            [synth stopSpeaking];
            [synth release];
            synth = nil;
        }
    }

    bool Speak(const char* text, bool interrupt) {
        if (!synth) return false;
        if (interrupt) [synth stopSpeaking];
        
        NSString* nsStr = [NSString stringWithUTF8String:text];
        return [synth startSpeakingString:nsStr] == YES;
    }

    bool Stop() {
        if (synth) [synth stopSpeaking];
        return true;
    }

    bool IsSpeaking() {
        return synth && [synth isSpeaking];
    }

    void SetVolume(int val) {
        this->volume = (float)val / 100.0f;
        if (synth) [synth setVolume:this->volume];
    }

    void SetRate(int val) {
        this->rate = (float)val;
        if (synth) [synth setRate:this->rate];
    }
};

namespace Sral {

	bool NsSpeech::Initialize() {
    obj = new NSSpeechSynthesizerWrapper();
    return obj->Initialize();
}

bool NsSpeech::Uninitialize() {
    if (obj) {
        obj->Uninitialize();
        delete obj;
        obj = nullptr;
    }
    return true;
}

bool NsSpeech::Speak(const char* text, bool interrupt) {
    return obj ? obj->Speak(text, interrupt) : false;
}

bool NsSpeech::StopSpeech() {
    return obj ? obj->Stop() : false;
}

bool NsSpeech::IsSpeaking() {
    return obj ? obj->IsSpeaking() : false;
}

bool NsSpeech::GetActive() {
    return obj != nullptr;
}

bool NsSpeech::SetParameter(int param, const void* value) {
    if (!obj) return false;
    int val = *reinterpret_cast<const int*>(value);
    switch (param) {
        case SRAL_PARAM_SPEECH_RATE:   obj->SetRate(val); break;
        case SRAL_PARAM_SPEECH_VOLUME: obj->SetVolume(val); break;
        default: return false;
    }
    return true;
}

bool NsSpeech::GetParameter(int param, void* value) {
    if (!obj) return false;
    switch (param) {
        case SRAL_PARAM_SPEECH_RATE:   *(int*)value = (int)obj->rate; break;
        case SRAL_PARAM_SPEECH_VOLUME: *(int*)value = (int)(obj->volume * 100); break;
        default: return false;
    }
    return true;
}

} // namespace Sral
