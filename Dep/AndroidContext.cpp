#include "AndroidContext.h"

namespace Sral {

namespace {
JavaVM* g_vm = nullptr;
jobject g_activity = nullptr;
}

bool SetAndroidJNIEnv(JNIEnv* env) {
	if (!env) return false;
	return env->GetJavaVM(&g_vm) == JNI_OK;
}

bool SetAndroidActivity(jobject activity) {
	if (!activity) return false;
	JNIEnv* env = GetAndroidJNIEnv();
	if (!env) return false;
	if (g_activity) {
		env->DeleteGlobalRef(g_activity);
		g_activity = nullptr;
	}
	g_activity = env->NewGlobalRef(activity);
	return g_activity != nullptr;
}

void ClearAndroidContext() {
	JNIEnv* env = GetAndroidJNIEnv();
	if (env && g_activity) {
		env->DeleteGlobalRef(g_activity);
	}
	g_activity = nullptr;
	g_vm = nullptr;
}

// A JNIEnv* is thread-local — the one passed to SetAndroidJNIEnv is only
// valid on the thread that called into SRAL. We cache the JavaVM*
// (process-global) instead and use GetEnv/AttachCurrentThread to obtain a
// valid JNIEnv* for whichever thread is currently calling into SRAL.
JNIEnv* GetAndroidJNIEnv() {
	if (!g_vm) return nullptr;
	JNIEnv* env = nullptr;
	jint status = g_vm->GetEnv((void**)&env, JNI_VERSION_1_6);
	if (status == JNI_OK) return env;
	if (status == JNI_EDETACHED) {
		if (g_vm->AttachCurrentThread(&env, nullptr) == JNI_OK) return env;
	}
	return nullptr;
}

jobject GetAndroidActivity() {
	return g_activity;
}

} // namespace Sral
