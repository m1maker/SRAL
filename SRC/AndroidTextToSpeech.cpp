  #include "AndroidTextToSpeech.h"
  #include <SDL3/SDL_system.h>  // For SDL_GetAndroidJNIEnv() and SDL_GetAndroidActivity()

  namespace Sral {

  bool AndroidTextToSpeech::Initialize() {
        env = (JNIEnv*)SDL_GetAndroidJNIEnv();
        if (!env) return false;

        jobject activity = (jobject)SDL_GetAndroidActivity();
        if (!activity) return false;

        speechClass = env->FindClass("org/sral/AndroidTTSHelper");
        if (!speechClass || env->ExceptionCheck()) {
                env->ExceptionClear();
                return false;
        }
        speechClass = (jclass)env->NewGlobalRef(speechClass);

        constructor = env->GetMethodID(speechClass, "<init>", "(Landroid/content/Context;)V");
        midSpeak = env->GetMethodID(speechClass, "speak", "(Ljava/lang/String;Z)V");
        midSilence = env->GetMethodID(speechClass, "stop", "()V");
        midIsActive = env->GetMethodID(speechClass, "isActive", "()Z");
        midIsSpeaking = env->GetMethodID(speechClass, "isSpeaking", "()Z");
        midSetRate = env->GetMethodID(speechClass, "setSpeechRate", "(F)V");
        midSetVolume = env->GetMethodID(speechClass, "setVolume", "(F)V");
        midGetRate = env->GetMethodID(speechClass, "getRate", "()F");
        midGetVolume = env->GetMethodID(speechClass, "getVolume", "()F");

        if (!constructor || !midSpeak || !midSilence || !midIsActive || !midIsSpeaking) return false;

        jobject localObj = env->NewObject(speechClass, constructor, activity);
        if (!localObj) return false;
        speechObj = env->NewGlobalRef(localObj);
        env->DeleteLocalRef(localObj);
        env->DeleteLocalRef(activity);

        return true;
  }

  bool AndroidTextToSpeech::Uninitialize() {
        ReleaseAllStrings();
        if (env && speechObj) {
                jmethodID midShutdown = env->GetMethodID(speechClass, "shutdown", "()V");
                if (midShutdown) env->CallVoidMethod(speechObj, midShutdown);
                env->DeleteGlobalRef(speechObj);
                speechObj = nullptr;
        }
        if (env && speechClass) {
                env->DeleteGlobalRef(speechClass);
                speechClass = nullptr;
        }
        return true;
  }

  bool AndroidTextToSpeech::GetActive() {
        if (!env || !speechObj || !midIsActive) return false;
        return env->CallBooleanMethod(speechObj, midIsActive);
  }

  bool AndroidTextToSpeech::Speak(const char* text, bool interrupt) {
        if (!env || !speechObj || !midSpeak) return false;
        jstring jtext = env->NewStringUTF(text);
        if (!jtext) return false;
        env->CallVoidMethod(speechObj, midSpeak, jtext, (jboolean)interrupt);
        env->DeleteLocalRef(jtext);
        return true;
  }

  bool AndroidTextToSpeech::StopSpeech() {
        if (!env || !speechObj || !midSilence) return false;
        env->CallVoidMethod(speechObj, midSilence);
        return true;
  }

  bool AndroidTextToSpeech::IsSpeaking() {
        if (!env || !speechObj || !midIsSpeaking) return false;
        return env->CallBooleanMethod(speechObj, midIsSpeaking);
  }

  bool AndroidTextToSpeech::SetParameter(int param, const void* value) {
        if (!env || !speechObj) return false;
        switch (param) {
        case SRAL_PARAM_SPEECH_RATE:
                if (midSetRate) {
                        env->CallVoidMethod(speechObj, midSetRate, (jfloat)*reinterpret_cast<const int*>(value));
                        return true;
                }
                return false;
        case SRAL_PARAM_SPEECH_VOLUME:
                if (midSetVolume) {
                        env->CallVoidMethod(speechObj, midSetVolume, (jfloat)*reinterpret_cast<const int*>(value));
                        return true;
                }
                return false;
        default:
                return false;
        }
  }

  bool AndroidTextToSpeech::GetParameter(int param, void* value) {
        if (!env || !speechObj) return false;
        switch (param) {
        case SRAL_PARAM_SPEECH_RATE:
                if (midGetRate) {
                        *(int*)value = (int)env->CallFloatMethod(speechObj, midGetRate);
                        return true;
                }
                return false;
        case SRAL_PARAM_SPEECH_VOLUME:
                if (midGetVolume) {
                        *(int*)value = (int)env->CallFloatMethod(speechObj, midGetVolume);
                        return true;
                }
                return false;
        default:
                return false;
        }
  }

  } // namespace Sral
