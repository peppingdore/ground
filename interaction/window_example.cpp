#include "window.h"
#include "../log.h"

int main() {
	auto [window, e] = create_window({ .title = U"Window"_b });
	if (e) {
		Log(e);
		return -1;
	}

	while (true) {
		auto events = read_window_events(window);
		for (auto event: events) {
			if (auto e = as<WindowCloseEvent>(event)) {
				return 0;
			}
		}
	}

	return 0;
}
