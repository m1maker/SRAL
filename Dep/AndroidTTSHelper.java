package org.sral;

import android.content.Context;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.speech.tts.UtteranceProgressListener;
import java.util.Locale;

public class AndroidTTSHelper {
		private TextToSpeech tts;
		private boolean ready = false;
		private boolean speaking = false;
		private float currentRate = 1.0f;
		private float currentVolume = 1.0f;

		public AndroidTTSHelper(Context context) {
				tts = new TextToSpeech(context, status -> {
						if (status == TextToSpeech.SUCCESS) {
								tts.setLanguage(Locale.getDefault());
								tts.setOnUtteranceProgressListener(new UtteranceProgressListener() {
										@Override public void onStart(String utteranceId) { speaking = true; }
										@Override public void onDone(String utteranceId) { speaking = false; }
										@Override public void onError(String utteranceId) { speaking = false; }
								});
								ready = true;
						}
				});
		}

		public boolean isActive() { return ready; }

		public boolean isSpeaking() { return speaking; }

		public void speak(String text, boolean interrupt) {
				if (!ready) return;
				int queueMode = interrupt ? TextToSpeech.QUEUE_FLUSH : TextToSpeech.QUEUE_ADD;
				Bundle params = new Bundle();
				tts.speak(text, queueMode, params, "sral_utterance");
		}

		public void stop() {
				if (tts != null) tts.stop();
				speaking = false;
		}

		public void setSpeechRate(float rate) {
				if (tts != null) tts.setSpeechRate(rate);
				currentRate = rate;
		}

		public void setVolume(float volume) {
				currentVolume = volume;
		}

		public float getRate() { return currentRate; }
		public float getVolume() { return currentVolume; }

		public void shutdown() {
				if (tts != null) {
						tts.stop();
						tts.shutdown();
						tts = null;
				}
				ready = false;
		}
}
