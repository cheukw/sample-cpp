#include "utils.h"

bool IsValidSocket(evutil_socket_t fd)
{
#ifdef _WIN32
	return (fd != INVALID_SOCKET);
#else  // _WIN32
	return (fd >= 0);
#endif  // else _WIN32
}

