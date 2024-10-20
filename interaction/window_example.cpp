#include "window.h"
#include "../grd_log.h"

int main() {
	auto [window, e] = create_window({ .title = U"Window"_b });
	if (e) {
		GrdLog(e);
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
