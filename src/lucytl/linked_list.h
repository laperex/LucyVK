#pragma once

#include <cstdint>


namespace ltl {
	template <typename T>
	class linked_list {
	public:	
		struct node {
			node* prev;
			node* next;

			T value;
		};

	private:
		node* _begin = nullptr;
		node* _end = nullptr;
		
		uint32_t _size = 0;


	public:
		void erase(node* _node) {
			if (_node != _end) {
				_node->next->prev = _node->prev;
			} else {
				_end = _node->prev;
			}

			if (_node != _begin) {
				_node->prev->next = _node->next;
			} else {
				_begin = _node->next;
			}
			
			_size--;

			delete _node;
		}

		node* push_back(const T& value) {
			node* new_node = new node {
				.prev = _end,
				.next = nullptr,
				.value = value
			};
			
			if (_end != nullptr) {
				_end->next = new_node;
			} else {
				_begin = new_node;
			}
			_end = new_node;
			
			_size++;
			
			return _end;
		}
		
		uint32_t size() const {
			return _size;
		}
		
		node* begin() const {
			return _begin;
		}
		
		node* end() const {
			return _end;
		}
		
		// T& data() const {
		// 	return _end;
		// }
	};
}