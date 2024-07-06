#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <vector>


struct lvk_destroyer {
	struct delete_element {
		std::vector<void*> data;
		std::size_t type;
	};

	std::map<void*, uint32_t> data_map;
	std::deque<delete_element> delete_queue;
	std::set<uint32_t> deleted_indices_set;
};
