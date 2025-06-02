#include "VoiceOver.h"

#if TARGET_OS_IOS || TARGET_OS_TV
#import <UIKit/UIKit.h>
#elif TARGET_OS_OSX
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#else
#error "Unsupported platform"
#endif

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

#if TARGET_OS_IOS || TARGET_OS_TV
		UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, msg);
#elif TARGET_OS_OSX
		NSDictionary* userInfo = @{ NSAccessibilityAnnouncementKeyStringValue: msg };
		NSAccessibilityPostNotificationWithUserInfo(NSApp, NSAccessibilityAnnouncementRequestedNotification, userInfo);
#endif
		(void)interrupt; // Unused yet
		return true;
	}

	bool VoiceOver::StopSpeech() {
		return Speak("", true);
	}

	bool VoiceOver::GetActive() {
#if TARGET_OS_IOS || TARGET_OS_TV
		return UIAccessibilityIsVoiceOverRunning() == YES ? true : false;
#elif TARGET_OS_OSX
		return [[NSWorkspace sharedWorkspace] isVoiceOverEnabled];
#endif
	}
}
