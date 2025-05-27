#include "AVSpeech.h"
#include <stdint.h>
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

class AVSpeechSynthesizerWrapper {
public:
	float rate;
	float volume;
	AVSpeechSynthesizer* synth;
	AVSpeechSynthesisVoice* currentVoice;
	AVSpeechUtterance* utterance;

	AVSpeechSynthesizerWrapper() : rate(0), volume(1), synth(nullptr), currentVoice(nullptr), utterance(nullptr) {}
AVSpeechSynthesisVoice* getVoiceObject(NSString* name){
 NSArray<AVSpeechSynthesisVoice*>* voices = [AVSpeechSynthesisVoice speechVoices];
 for (AVSpeechSynthesisVoice* v in voices) {
  if ([v.name isEqualToString : name]) return v;
 }
 return nil;
}

bool Initialize() {
 currentVoice = [AVSpeechSynthesisVoice voiceWithLanguage:@"en-US"]; //choosing english as a default language
 utterance = [[AVSpeechUtterance alloc] initWithString:@""];
 rate = utterance.rate;
 volume = utterance.volume;
 synth = [[AVSpeechSynthesizer alloc] init];
 return true;
}
bool Uninitialize(){
 return true;
}
bool Speak(const char* text, bool interrupt){
 if (interrupt && synth.isSpeaking)[synth stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
 NSString *nstext = [NSString stringWithUTF8String:text];
 AVSpeechUtterance *utterance = [[AVSpeechUtterance alloc] initWithString:nstext];
 utterance.rate = rate;
 utterance.volume = volume;
 utterance.voice = currentVoice;
 this->utterance = utterance;
 [synth speakUtterance:this->utterance];
 return synth.isSpeaking;
}
bool StopSpeech(){
 if (synth.isSpeaking) return [synth stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
 return false;
}

bool IsSpeaking() {
	return synth.isSpeaking;
}
bool GetActive(){
 return synth != nil;
}
void SetVolume(int value){
 this->volume = value;
}
int GetVolume(){
 return this->volume;
}
void SetRate(int value){
 this->rate = value;
}
int GetRate(){
 return this->rate;
}
uint64_t GetVoiceCount(){
 NSArray<AVSpeechSynthesisVoice *> *voices = [AVSpeechSynthesisVoice speechVoices];
 return voices.count;
}
const char* GetVoiceName(uint64_t index){
 NSArray<AVSpeechSynthesisVoice *> *voices = [AVSpeechSynthesisVoice speechVoices];
 @try {
  return [[voices objectAtIndex:index].name UTF8String];
 } @catch (NSException *exception) {
  return "";
 }
}
bool SetVoice(uint64_t index){
 NSArray<AVSpeechSynthesisVoice *> *voices = [AVSpeechSynthesisVoice speechVoices];
 AVSpeechSynthesisVoice *oldVoice = currentVoice;
 @try {
  currentVoice = [voices objectAtIndex:index];
  return true;
 } @catch (NSException *exception) {
  currentVoice = oldVoice;
  return false;
 }
}
};
namespace Sral {

bool AvSpeech::Initialize() {
	obj = new AVSpeechSynthesizerWrapper();
	return obj->Initialize();
}

bool AvSpeech::Uninitialize() {
	if (obj == nullptr) return false; // Check for nullptr
	delete obj;
	obj = nullptr; // Set to nullptr after deletion
	return true; // Return true to indicate successful uninitialization
}

bool AvSpeech::GetActive() {
	return obj != nullptr && obj->GetActive();
}

bool AvSpeech::Speak(const char* text, bool interrupt) {
	return obj->Speak(text, interrupt);
}

bool AvSpeech::StopSpeech() {
	return obj->StopSpeech();
}
bool AvSpeech::IsSpeaking() {
	return obj->IsSpeaking();
}

bool AvSpeech::SetParameter(int param, const void* value) {
	switch (param) {
	case SRAL_PARAM_SPEECH_RATE:
		obj->SetRate(*reinterpret_cast<const int*>(value));
		return true;
	case SRAL_PARAM_SPEECH_VOLUME:
		obj->SetVolume(*reinterpret_cast<const int*>(value));
		return true;
	case SRAL_PARAM_VOICE_INDEX:
		return obj->SetVoice(*reinterpret_cast<const int*>(value));
	default:
		return false;
	}
	return true;
}

bool AvSpeech::GetParameter(int param, void* value) {
	switch (param) {
	case SRAL_PARAM_SPEECH_RATE: {
		*(int*)value = obj->GetRate();
		return true;
	}
	case SRAL_PARAM_SPEECH_VOLUME: {
		*(int*)value = obj->GetVolume();
		return true;
	}
	case SRAL_PARAM_VOICE_LIST: {
		int voice_count = obj->GetVoiceCount();
		char** voices = (char**)value;
		for (int i = 0; i < voice_count; ++i) {
			const char* desc = obj->GetVoiceName(i);
			strcpy(voices[i], desc);
		}
	return true;
	}
	default:
		return false;
	}
	return true;
}

}
