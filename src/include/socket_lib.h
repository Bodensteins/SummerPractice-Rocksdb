#ifndef _MYSOCKLIB_H_
#define  _MYSOCKLIB_H_

int make_server_socket(int portnum);

int make_server_socket_q(int portnum, int backlog);

int connect_to_server(const char *host, int portnum);

#endif
