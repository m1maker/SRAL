#ifndef SRAL_ANDROID_CONTEXT_H_
#define SRAL_ANDROID_CONTEXT_H_
#pragma once
#include <jni.h>

// Shared Android JNI context for SRAL engines.
//
// Native code on Android cannot obtain a JNIEnv* or the app's Activity on its
// own — the host application must provide them. The host sets them via
// SRAL_SetEngineParameter using SRAL_PARAM_ANDROID_JNI_ENV and
// SRAL_PARAM_ANDROID_ACTIVITY (see SRAL.h), which forward into the setters
// below. Engine implementations (e.g. AndroidTextToSpeech, future
// AndroidTalkBack) retrieve the values through the accessor functions.
//
// SetAndroidJNIEnv must be called before SetAndroidActivity, since creating
// a global ref for the activity requires a valid JNIEnv*.

namespace Sral {

// Captures the JavaVM* from the provided env. Returns false if env is null.
bool SetAndroidJNIEnv(JNIEnv* env);

// Stores a global ref to activity. Requires SetAndroidJNIEnv to have been
// called first. Returns false if the JavaVM is not yet set, activity is null,
// or the global ref could not be created.
bool SetAndroidActivity(jobject activity);

// Called by SRAL_Uninitialize. Releases the global ref to activity.
void ClearAndroidContext();

// Returns a JNIEnv* valid on the calling thread, attaching the thread to
// the JavaVM if necessary. Returns nullptr if SetAndroidJNIEnv has not
// been called or if attach fails.
JNIEnv* GetAndroidJNIEnv();

// Returns the Activity global ref, or nullptr if SetAndroidActivity has
// not been called.
jobject GetAndroidActivity();

} // namespace Sral

#endif
