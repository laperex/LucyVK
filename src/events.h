#pragma once

namespace lucy {
	class events {
		bool is_quit;

	public:
		bool& quit();
	};
}