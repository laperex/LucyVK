#pragma once

#define LVK_EXCEPT(assertion, message)	\
	if ((assertion) != true)	\
		throw std::runtime_error(message);

#define LVK_TIMEOUT 1000000000