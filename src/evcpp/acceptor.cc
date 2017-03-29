#include "acceptor.h"
#include "connection.h"
#include "utils.h"

Acceptor::Acceptor(struct event_base* ev_base, 
	const std::string& ip, 
	uint16_t port, 
	AcceptCallbacksPtr cb, 
	ConnCallbacksPtr lkcb)
	: ev_base_(ev_base)
	, listener_(nullptr)
	, accept_cbs_(cb)
	, conn_cbs_(lkcb)
	, is_listening_(false)
	, ip_(ip)
	, port_(port)
{

}

Acceptor::~Acceptor()
{
	if (listener_ != nullptr) {
		evconnlistener_free(listener_);
		listener_ = nullptr;
	}
}

bool Acceptor::Listen() {
	// just listen on IPv4
	evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (!IsValidSocket(fd)) {
		int err = EVUTIL_SOCKET_ERROR();
		LOG_ERROR("[error] err:%d %s\n", err, evutil_socket_error_to_string(err));
		return false;
	}

	if (evutil_make_listen_socket_reuseable(fd) != 0){
		int err = EVUTIL_SOCKET_ERROR();
		LOG_ERROR("[error] make socket reuseable err:%d %s\n", err, evutil_socket_error_to_string(err));
		evutil_closesocket(fd);
		return false;
	}

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	if (evutil_inet_pton(AF_INET, ip_.c_str(), &sa.sin_addr) != 1) {
		int err = EVUTIL_SOCKET_ERROR();
		LOG_ERROR("[error] inet_pton err:%d %s\n", err, evutil_socket_error_to_string(err));
		evutil_closesocket(fd);
		return false;
	}
	sa.sin_family = AF_INET;
	sa.sin_port = ntohs(port_);

	if (bind(fd, reinterpret_cast<struct sockaddr*>(&sa), sizeof(struct sockaddr_in)) != 0) {
		int err = EVUTIL_SOCKET_ERROR();
		LOG_ERROR("[error] bind address err:%d %s\n", err, evutil_socket_error_to_string(err));
		evutil_closesocket(fd);
		return false;
	}

	if (evutil_make_socket_nonblocking(fd) != 0)
	{
		int err = EVUTIL_SOCKET_ERROR();
		LOG_ERROR("[error] non-blocking err:%d %s\n", err, evutil_socket_error_to_string(err));
		evutil_closesocket(fd);
		return false;
	}

	if (listen(fd, 0) != 0)
	{
		int err = EVUTIL_SOCKET_ERROR();
		LOG_ERROR("[error] listen err:%d %s\n", err, evutil_socket_error_to_string(err));
		evutil_closesocket(fd);
		return false;
	}

	listener_ = evconnlistener_new(ev_base_, AcceptCallback, this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 0, fd);
	is_listening_ = true;

	return true;
}
void Acceptor::Close() {
	evconnlistener_disable(listener_);
	is_listening_ = false;
}

void Acceptor::AcceptCallback(evconnlistener * listener, evutil_socket_t fd, sockaddr* addr, int len, void* ctx) {
	ASSERT(ctx != nullptr);
	Acceptor* self = reinterpret_cast<Acceptor*>(ctx);
	ASSERT(self != nullptr);
	ASSERT(listener == self->listener_);

	if (self->conn_cbs_ == nullptr) {
		evutil_closesocket(fd);
		return;
	}

	auto conn = std::make_shared<Connection>(self->ev_base_, fd, self->conn_cbs_);
	conn->Enable();

	if (self->accept_cbs_) {
		self->accept_cbs_->OnAccepted(conn);
	}
}

