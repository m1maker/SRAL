package org.sral;

import android.content.Context;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;

public class AndroidAccessibilityManagerHelper {
	private final Context context;
	private final AccessibilityManager am;

	public AndroidAccessibilityManagerHelper(Context context) {
		this.context = context;
		this.am = (AccessibilityManager)
			context.getSystemService(Context.ACCESSIBILITY_SERVICE);
	}

	public boolean isActive() {
		return am != null && am.isEnabled() && am.isTouchExplorationEnabled();
	}

	public void announce(String text, boolean interrupt) {
		if (am == null || !am.isEnabled() || text == null) return;
		if (interrupt) am.interrupt();
		AccessibilityEvent event = AccessibilityEvent.obtain();
		event.setEventType(AccessibilityEvent.TYPE_ANNOUNCEMENT);
		event.setPackageName(context.getPackageName());
		event.setClassName(AndroidAccessibilityManagerHelper.class.getName());
		event.getText().add(text);
		am.sendAccessibilityEvent(event);
	}

	public void stop() {
		if (am != null) am.interrupt();
	}

	public void shutdown() {
	}
}
