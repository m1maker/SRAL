#include "AndroidAccessibilityManager.h"
#include "../Dep/AndroidContext.h"

namespace Sral {

bool AndroidAccessibilityManager::Initialize() {
	env = GetAndroidJNIEnv();
	if (!env) return false;

	jobject activity = GetAndroidActivity();
	if (!activity) return false;

	announcerClass = env->FindClass("org/sral/AndroidAccessibilityManagerHelper");
	if (!announcerClass || env->ExceptionCheck()) {
		env->ExceptionClear();
		return false;
	}
	announcerClass = (jclass)env->NewGlobalRef(announcerClass);

	constructor = env->GetMethodID(announcerClass, "<init>", "(Landroid/content/Context;)V");
	midIsActive = env->GetMethodID(announcerClass, "isActive", "()Z");
	midAnnounce = env->GetMethodID(announcerClass, "announce", "(Ljava/lang/String;Z)V");
	midStop = env->GetMethodID(announcerClass, "stop", "()V");
	midShutdown = env->GetMethodID(announcerClass, "shutdown", "()V");

	if (!constructor || !midIsActive || !midAnnounce || !midStop) return false;

	jobject localObj = env->NewObject(announcerClass, constructor, activity);
	if (!localObj) return false;
	announcerObj = env->NewGlobalRef(localObj);
	env->DeleteLocalRef(localObj);

	return true;
}

bool AndroidAccessibilityManager::Uninitialize() {
	ReleaseAllStrings();
	if (env && announcerObj) {
		if (midShutdown) env->CallVoidMethod(announcerObj, midShutdown);
		env->DeleteGlobalRef(announcerObj);
		announcerObj = nullptr;
	}
	if (env && announcerClass) {
		env->DeleteGlobalRef(announcerClass);
		announcerClass = nullptr;
	}
	return true;
}

bool AndroidAccessibilityManager::GetActive() {
	if (!env || !announcerObj || !midIsActive) return false;
	return env->CallBooleanMethod(announcerObj, midIsActive);
}

bool AndroidAccessibilityManager::Speak(const char* text, bool interrupt) {
	if (!env || !announcerObj || !midAnnounce) return false;
	jstring jtext = env->NewStringUTF(text);
	if (!jtext) return false;
	env->CallVoidMethod(announcerObj, midAnnounce, jtext, (jboolean)interrupt);
	env->DeleteLocalRef(jtext);
	return true;
}

bool AndroidAccessibilityManager::StopSpeech() {
	if (!env || !announcerObj || !midStop) return false;
	env->CallVoidMethod(announcerObj, midStop);
	return true;
}

} // namespace Sral
