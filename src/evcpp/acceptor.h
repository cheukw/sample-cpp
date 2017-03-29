#pragma once
#ifndef _LISTENER_H_
#define _LISTENER_H_
#include "common.h"
#include "callbacks.h"

class Acceptor {
public:
	Acceptor(struct event_base* ev_base, 
		const std::string& ip,	
		uint16_t port, 
		AcceptCallbacksPtr cb, 
		ConnCallbacksPtr lkcb);

	~Acceptor();

	bool Listen();
	void Close();
	bool IsListening() const { return is_listening_; }
private:
	static void AcceptCallback(evconnlistener * listener, evutil_socket_t fd, sockaddr* addr, int len, void* ctx);

private:
	struct event_base* ev_base_;
	struct evconnlistener* listener_;

	AcceptCallbacksPtr accept_cbs_;
	ConnCallbacksPtr conn_cbs_;

	bool is_listening_;

	std::string ip_;
	uint16_t port_;
};
#endif /// _LISTENER_H_

