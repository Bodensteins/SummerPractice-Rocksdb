#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/socket_lib.h"

int clnt_port=32000;
int srv_port=35000;
char buf[BUFSIZ];

int sock_id;
int fd_clnt;
int fd_srv;
int n;

int main(){
    sock_id=make_server_socket(clnt_port);

    while(true){
        fd_clnt=accept(sock_id,nullptr,nullptr);

        fd_srv=connect_to_server("bodensteins",srv_port);

        do{
            n=read(fd_clnt,buf,BUFSIZ);
            write(fd_srv,buf,n);
        }while(n>0);

        printf("proxy finish reading\n");

        do{
            n=read(fd_srv,buf,BUFSIZ);
            write(fd_clnt,buf,n);
        }while(n>0);
        
        printf("proxy finish writing\n");

        close(fd_srv);
    }

    return 0;
}