#include "ACAnnouncer.h"
#include <iostream>

bool ACAnnouncer::Speak(const char* text, bool interrupt) {
	static accesskit_tree tree = accesskit_tree_new(WINDOW_ID);
	static accesskit_windows_adapter adapter = accesskit_windows_adapter_new(GetForegroundWindow(), true, [](struct accesskit_action_request* request, void* userdata) {
		accesskit_action_request_free(request);
			}, nullptr);

	accesskit_node* window_node = accesskit_node_new(ACCESSKIT_ROLE_WINDOW);
	accesskit_node* announcement_node = accesskit_node_new(ACCESSKIT_ROLE_LABEL);
	accesskit_tree_update* update = accesskit_tree_update_with_capacity_and_focus(2, ANNOUNCEMENT_ID);

	accesskit_node_set_label(announcement_node, text);

	if (interrupt) {
		accesskit_node_set_live(announcement_node, ACCESSKIT_LIVE_ASSERTIVE);
	}
	else {
		accesskit_node_set_live(announcement_node, ACCESSKIT_LIVE_POLITE);
	}
	accesskit_node_push_child(window_node, ANNOUNCEMENT_ID);

	accesskit_tree_update_push_node(update, WINDOW_ID, window_node);
	accesskit_tree_update_push_node(update, ANNOUNCEMENT_ID, announcement_node);

	accesskit_windows_adapter_update_if_active(&adapter, [](void* userdata) {
		return (accesskit_tree_update*)userdata;
			}, update);

	return true;
}

bool ACAnnouncer::StopSpeech() {
	return Speak(true);
}

bool ACAnnouncer::GetActive() {
	return true;
}

bool ACAnnouncer::Initialize() {
	return true;
}

bool ACAnnouncer::Uninitialize() {
	return true;
}
