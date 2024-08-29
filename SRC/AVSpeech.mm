#include "AVSpeech.h"
#include <stdint.h>
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

AVSpeechSynthesisVoice* AVSpeech::getVoiceObject(NSString* name){
	NSArray<AVSpeechSynthesisVoice*>* voices = [AVSpeechSynthesisVoice speechVoices];
	for (AVSpeechSynthesisVoice* v in voices) {
		if ([v.name isEqualToString : name]) return v;
	}
	return nil;
}

bool AVSpeech::Initialize() {
	currentVoice = [AVSpeechSynthesisVoice voiceWithLanguage:@"en-US"]; //choosing english as a default language
	utterance = [[AVSpeechUtterance alloc] initWithString:@""];
	rate = utterance.rate;
	volume = utterance.volume;
	synth = [[AVSpeechSynthesizer alloc] init];
	return true;
}
bool AVSpeech::Uninitialize(){
	return true;
}
bool AVSpeech::Speak(const char* text, bool interrupt){
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
bool AVSpeech::StopSpeech(){
	if (synth.isSpeaking) return [synth stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
	return false;
}
bool AVSpeech::GetActive(){
	return synth != nil;
}
void AVSpeech::SetVolume(uint64_t value){
	this->volume = value;
}
uint64_t AVSpeech::GetVolume(){
	return this->volume;
}
void AVSpeech::SetRate(uint64_t value){
	this->rate = value;
}
uint64_t AVSpeech::GetRate(){
	return this->rate;
}
uint64_t AVSpeech::GetVoiceCount(){
	NSArray<AVSpeechSynthesisVoice *> *voices = [AVSpeechSynthesisVoice speechVoices];
	return voices.count;
}
const char* AVSpeech::GetVoiceName(uint64_t index){
	NSArray<AVSpeechSynthesisVoice *> *voices = [AVSpeechSynthesisVoice speechVoices];
	@try {
		return [[voices objectAtIndex:index].name UTF8String];
	} @catch (NSException *exception) {
		return "";
	}
}
bool AVSpeech::SetVoice(uint64_t index){
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
