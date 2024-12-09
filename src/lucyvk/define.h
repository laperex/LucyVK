#pragma once

#define LVK_EXCEPT(assertion, message)	\
	if ((assertion) != true)	\
		throw std::runtime_error(message);

#define LVK_TIMEOUT 1000000000

// #define LVK_HANDLE_DEF_CONVERT(h_type, c_type, h_name) h_type h_name; operator c_type() const { return h_name; }
#define LVK_HANDLE_DEF(h_type, h_name) h_type h_name; operator decltype(h_name)() const { return h_name; }

// #define LVK_ASSERT(t) assert(t)
