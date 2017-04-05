#ifndef SOCKET_OPS_H_
#define SOCKET_OPS_H_

int make_socket_non_blocking(int fd);

int make_socket_reuse(int fd); 

int create_and_bind(const char* ip, unsigned short port);

#endif /// SOCKET_OPS_H_
