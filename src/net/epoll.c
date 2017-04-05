#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "socket_ops.h"

#define EPOLL_EVENT_SIZE 64
#define MAX_BUFFER_SIZE 1024

void do_epoll(int listenfd);
void do_read(int efd, int fd);
void do_write(int efd, int fd);

void handle_accept(int efd, int listenfd);
void handle_events(int efd, struct epoll_event* events, int num, int listenfd);

void add_event(int efd, int fd, int state);
void modify_event(int efd, int fd, int state);
void delete_event(int efd, int fd, int state);

int mul_fd = -1;

int main()
{
	int listenfd;
	char ip[] = "0.0.0.0";
	listenfd = create_and_bind(ip, 7788);
	listen(listenfd, 5);
	do_epoll(listenfd);

	return 0;
}



void do_epoll(int listenfd)
{
	int efd;
	struct epoll_event events[EPOLL_EVENT_SIZE];
	int ret;
	
	efd = epoll_create(1000);
	add_event(efd, listenfd, EPOLLIN);

	while(1) 	{
		ret = epoll_wait(efd, events, 1000, -1);
		handle_events(efd, events, ret, listenfd);
	}

	close(efd);
}

void handle_events(int efd, struct epoll_event* events, int num, int listenfd)
{
	int i, fd;
	for (int i = 0; i < num; ++i)	{
		fd = events[i].data.fd;
		if((fd == listenfd) && (events[i].events & EPOLLIN))
			handle_accept(efd, listenfd);
		else if(events[i].events & EPOLLIN)
			do_read(efd, fd);
		else if(events[i].events & EPOLLOUT)
			do_write(efd, fd);
	}
}


void handle_accept(int efd, int listenfd)
{
	int clifd;
	struct sockaddr_in cliaddr;
	socklen_t clisocklen;
	clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &clisocklen);
	if(-1 == clifd)
		perror("accept error");
	else
	{
		printf("accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
		make_socket_non_blocking(clifd);
		// add_event(efd, clifd, EPOLLIN | EPOLLET);
		// add_event(efd, clifd, EPOLLIN | EPOLLOUT);
		add_event(efd, clifd, EPOLLIN);
		if(mul_fd == -1) {
			mul_fd = clifd;
		}
	}
}

void do_read(int efd, int fd)
{
	int nread;
	char buf[MAX_BUFFER_SIZE];
	
	while(1) {
		memset(buf, 0, MAX_BUFFER_SIZE);
		nread = read(fd, buf, MAX_BUFFER_SIZE);
		if(nread < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				perror("read EAGAIN error.");
				break;
			}
			perror("read error.");
			close(fd);
			delete_event(efd, fd, EPOLLIN);
			return;
		}

		if(0 == nread) {
			fprintf(stderr, "client close. fd: %d\n", fd);
			close(fd);
			delete_event(efd, fd, EPOLLIN);
			return;
		}
		// if(mul_fd != fd) {
		// 	// printf("%s\n", "1111111111111111111");
		// } else {
		// 	//printf("%s\n", "2");
		// }
		
		if (nread < MAX_BUFFER_SIZE) {
			break;
		}
		
	}
	
}
void do_write(int efd, int fd)
{
		
}

void add_event(int efd, int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
}

void modify_event(int efd, int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(efd, EPOLL_CTL_MOD, fd, &ev);
}

void delete_event(int efd, int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ev);
}


