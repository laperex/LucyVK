#pragma once

namespace ltl {
	template <typename T>
	struct heap_array {
		T* _begin;
		T* _end;

		T* data() {
			return _begin;
		}
	};
}