#pragma once

#define LVK_EXCEPT(assertion, message)	\
	if ((assertion) != true)	\
		throw std::runtime_error(message);

#define LVK_TIMEOUT 1000000000

#define LVK_HANDLE_DEF(h_type, h_name) h_type h_name; operator decltype(h_name)() const { return h_name; }
