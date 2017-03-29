#pragma once
#ifndef _COMMON_H_
#define _COMMON_H_

#include <cstdint>
#include <memory>
#include <cassert>
#include <string>
#include <functional>
#include <thread>
#include <vector>


#if defined(_WIN32) || defined(_WIN64) 
#include <WinSock2.h>
#include <WS2tcpip.h>

#define usleep(us) Sleep((us)/1000)
#define snprintf  _snprintf

// warning C4005 : 'va_copy' : macro redefinition
#pragma warning( disable: 4005 ) 
#pragma warning( disable: 4251 )

// warning C4996: 'strerror': This function or variable may be unsafe. 
// Consider using strerror_s instead. To disable deprecation, 
// use _CRT_SECURE_NO_WARNINGS. See online help for details.
#pragma warning( disable: 4996 ) 
#pragma warning( disable: 4244 4251 4355 4715 4800 4996 4005 4819)
#pragma warning( disable: 4267 ) 

#else
#include <sys/queue.h>
#endif

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/http_compat.h>
#include <event2/http_struct.h>
#include <event2/event_compat.h>
#include <event2/dns.h>
#include <event2/dns_compat.h>
#include <event2/dns_struct.h>
#include <event2/listener.h>


#define ASSERT(exp) assert(exp)

#ifdef _DEBUG
#define LOG_INFO printf
#define LOG_ERROR printf
#define LOG_WARNING printf
#define LOG_FATAL printf

#else
#define LOG_INFO(...)
#define LOG_ERROR(...)
#define LOG_WARNING(...)
#define LOG_FATAL(...)
#endif

class Connection;
typedef std::shared_ptr<Connection> TCPConnPtr;

class Acceptor;
typedef std::shared_ptr<Acceptor> AcceptorPtr;

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;
#endif // _COMMON_H_

