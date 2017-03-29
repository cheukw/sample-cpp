#include "utils.h"
#include "connector.h"
#include "connection.h"


Connector::Connector(struct event_base* ev_base, 
	const std::string& ip, 
	uint16_t port, 
	ConnectCallbacksPtr cb, 
	ConnCallbacksPtr lkcb,
	bool reconnect)
	: ev_base_(ev_base)
	, callbacks_(cb)
	, conn_cbs_(lkcb)
	, ip_(ip)
	, port_(port)
	, closed_(false)
{

}

Connector::~Connector()
{
	if (!closed_) {
		Close();
	}
}

bool Connector::Connect()
{
	evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (!IsValidSocket(fd)) {
		int err = EVUTIL_SOCKET_ERROR();
		LOG_ERROR("[error] err:%d %s\n", err, evutil_socket_error_to_string(err));
		return false;
	}

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_port = ::htons(port_);
	if (evutil_inet_pton(AF_INET, ip_.c_str(), &sa.sin_addr)  != 1) {
		return false;
	}
	int salen = static_cast<int>(sizeof(sa));

	if (::connect(fd, (const sockaddr*)&sa, salen) != 0) {
		int err = EVUTIL_SOCKET_ERROR();
		LOG_ERROR("[error] err:%d %s\n", err, evutil_socket_error_to_string(err));
		return false;
	}
	
	conn_ = std::make_shared<Connection>(ev_base_, fd, conn_cbs_);
	conn_->Enable();
	callbacks_->OnConnected(conn_);

	return true;
}

bool Connector::AsyncConnect()
{
	TCPConnPtr conn = Connection::Connect(ev_base_, ip_, port_, shared_from_this(), conn_cbs_);
	if (conn == nullptr) {
		return false;
	}
	conn_ = conn;
	return true;
}

void Connector::Close() {
	closed_ = true;
	conn_->Close();
	
}

void Connector::OnClose() {
	LOG_INFO("conn close");
	conn_.reset();
}