#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "socket_ops.h"

int make_socket_non_blocking(int fd)
{
	int flags, s;
	flags = fcntl(fd, F_GETFL, 0);
	if (-1 == flags)	{
		perror("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(fd, F_SETFL, flags);
	if (-1 == s)	{
		perror("fcntl");
		return -1;
	}
	return 0;
}

int make_socket_reuse(int fd)
{
	int optval = 1;
	socklen_t len = (socklen_t)sizeof(optval);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, len);
	optval = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, len);
}

int create_and_bind(const char* ip, unsigned short port)
{
	int listenfd;
	struct sockaddr_in servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == listenfd) {
		perror("socket error.");
		exit(1);
	}
	bzero(&servaddr, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
	{
		perror("bind error.");
		exit(1);
	}
	make_socket_non_blocking(listenfd);
	make_socket_reuse(listenfd);
	return listenfd;
}