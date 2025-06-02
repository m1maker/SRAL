#include "VoiceOver.h"
#import <UIKit/UIKit.h>

namespace Sral {
	bool VoiceOver::Initialize() {
		return true;
	}

	bool VoiceOver::Uninitialize() {
		return true;
	}

	bool VoiceOver::Speak(const char* text, bool interrupt) {
		if (!text) {
			return false;
		}
		NSString* msg = [NSString stringWithUTF8String:text];
		UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, msg);
		(void)interrupt; // Unused yet
		return true;
	}

	bool VoiceOver::StopSpeech() {
		return Speak("", true);
	}
	bool VoiceOver::GetActive() {
		return UIAccessibilityIsVoiceOverRunning() == YES ? true : false;
	}
}
