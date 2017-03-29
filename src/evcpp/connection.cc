#include "connection.h"
#include "connector.h"

Connection::Connection(struct event_base* ev_base, evutil_socket_t fd, const ConnCallbacksPtr& cb)
	: bev_(bufferevent_socket_new(ev_base, fd, BEV_OPT_CLOSE_ON_FREE))
	, self_(nullptr)
	, callbacks_(cb)
	, connector_(nullptr)
	, local_ip_()
	, local_port_(0)
	, remote_ip_()
	, remote_port_(0)
	, enabled_(false)
	, closing_(false)
{

}

Connection::Connection(struct event_base* ev_base,
	const ConnectorPtr& connector,
	const ConnCallbacksPtr& lkcb)
	: bev_(bufferevent_socket_new(ev_base, -1, BEV_OPT_CLOSE_ON_FREE))
	, self_(nullptr)
	, callbacks_(lkcb)
	, connector_(connector)
	, local_ip_()
	, local_port_(0)
	, remote_ip_()
	, remote_port_(0)
	, enabled_(false)
	, closing_(false)
{
}

TCPConnPtr Connection::Connect(struct event_base* ev_base, 
	const std::string& ip,
	uint16_t port,
	const ConnectorPtr& connector,
	const ConnCallbacksPtr& lkcb) 
{
	auto link = TCPConnPtr(new Connection(ev_base, connector, lkcb));
	link->Enable();
	if (!link->Connect(ip, port)) {
		link->Close();
		return nullptr;
	}
	return link;
}

bool Connection::Connect(const std::string& ip, uint16_t port) {
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_port = ::htons(port);
	if (evutil_inet_pton(AF_INET, ip.c_str(), &sa.sin_addr) != 1) {
		return false;
	}
	int salen = static_cast<int>(sizeof(sa));
	return (bufferevent_socket_connect(bev_, (struct sockaddr*)&sa, salen) == 0);
}

Connection::~Connection() {
	bufferevent_free(bev_);
	bev_ = nullptr;
}

void Connection::Enable() {
	self_ = shared_from_this();
	bufferevent_setcb(bev_, ReadCallback, WriteCallback, EventCallback, this);
	bufferevent_enable(bev_, EV_READ | EV_WRITE);
	enabled_ = true;
}

bool Connection::IsEnabled() const {
	return enabled_;
}

bool Connection::Send(const char* data, uint32_t len) {
	return bufferevent_write(bev_, data, len) == 0;
}

void Connection::Shutdown() {
	if (evbuffer_get_length(bufferevent_get_output(bev_)) == 0) {
		DoActualShutdown();
		return;
	}

	closing_ = true;
}

void Connection::Close() {
	bufferevent_disable(bev_, EV_READ | EV_WRITE);
	self_.reset();
}

void Connection::UpdateLocalAddress() {
	sockaddr_storage sa;
	socklen_t salen = static_cast<socklen_t>(sizeof(sa));
	getsockname(bufferevent_getfd(bev_), reinterpret_cast<sockaddr *>(&sa), &salen);
	UpdateAddress(reinterpret_cast<const sockaddr *>(&sa), salen, local_ip_, local_port_);
}

void Connection::UpdateRemoteAddress() {
	sockaddr_storage sa;
	socklen_t salen = static_cast<socklen_t>(sizeof(sa));
	getpeername(bufferevent_getfd(bev_), reinterpret_cast<sockaddr *>(&sa), &salen);
	UpdateAddress(reinterpret_cast<const sockaddr *>(&sa), salen, remote_ip_, remote_port_);
}

void Connection::UpdateAddress(const sockaddr* addr, socklen_t a_AddrLen, std::string& ip, uint16_t& port) {
	char IP[128];
	switch (addr->sa_family) {
	case AF_INET:  // IPv4:
	{
		const sockaddr_in * sin = reinterpret_cast<const sockaddr_in*>(addr);
		evutil_inet_ntop(AF_INET, &(sin->sin_addr), IP, sizeof(IP));
		port = ntohs(sin->sin_port);
		break;
	}
	case AF_INET6:  // IPv6
	{
		const sockaddr_in6 * sin = reinterpret_cast<const sockaddr_in6 *>(addr);
		evutil_inet_ntop(AF_INET6, &(sin->sin6_addr), IP, sizeof(IP));
		port = ntohs(sin->sin6_port);
		break;
	}
	default: {	break; }
	}
	ip.assign(IP);
}

void Connection::DoActualShutdown() {
#ifdef _WIN32
	shutdown(bufferevent_getfd(bev_), SD_SEND);
#else
	shutdown(bufferevent_getfd(bev_), SHUT_WR);
#endif
	bufferevent_disable(bev_, EV_WRITE);
}


void Connection::ReadCallback(struct bufferevent* bev, void *ctx) {
	ASSERT(ctx != nullptr);
	Connection* self = static_cast<Connection*>(ctx);
	ASSERT(self->bev_ == bev);
	ASSERT(self->callbacks_ != nullptr);

	char data[1024];
	std::size_t length = 0;
	while ((length = bufferevent_read(bev, data, 1024)) > 0) {
		self->callbacks_->OnReceivedData(data, length);
	}
}
void Connection::WriteCallback(struct bufferevent* bev, void * ctx) {
	ASSERT(ctx != nullptr);
	Connection* self = static_cast<Connection*>(ctx);
	ASSERT(self->bev_ == bev);
	ASSERT(self->callbacks_ != nullptr);

	auto length = evbuffer_get_length(bufferevent_get_output(bev));
	if ((length == 0) && (self->closing_))
	{
		self->Shutdown();
	}
}
void Connection::EventCallback(struct bufferevent* bev, short what, void *ctx) {
	ASSERT(ctx != nullptr);
	TCPConnPtr self = static_cast<Connection*>(ctx)->self_;
	ASSERT(self->bev_ == bev);
	ASSERT(self->callbacks_ != nullptr);

	if (what & BEV_EVENT_ERROR) {
		int err = EVUTIL_SOCKET_ERROR();
		self->callbacks_->OnError(err, evutil_socket_error_to_string(err));

		if (self->connector_) {
			self->connector_->callbacks_->OnError(err, evutil_socket_error_to_string(err));
		}
		self->self_.reset();
		return;
	}

	if (what & BEV_EVENT_CONNECTED) {
		self->UpdateLocalAddress();
		self->UpdateRemoteAddress();
		if (self->connector_) {
			self->connector_->callbacks_->OnConnected(self);
		}
		return;
	}

	if (what & BEV_EVENT_EOF) {
		self->callbacks_->OnRemoteClosed();
		if (self->connector_) {
			self->connector_->OnClose();
		}
		self->self_.reset();
	}
}

