#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/socket_lib.h"

int portnum=35000;
int sock_id;
int fd;
char input1[BUFSIZ];
char input2[BUFSIZ];
char sz1[11];
char sz2[11];
const int lsize=11;

//char sz3[11];
std::string output("test output yes");
//size_t sz1,sz2;

int main(){
    sock_id=make_server_socket(portnum);
    while (true){
        fd=accept(sock_id,nullptr,nullptr);
        //accept meta data and input
        memset(input1,0,BUFSIZ);
        memset(input2,0,BUFSIZ);
        memset(sz1,0,lsize);
        memset(sz2,0,lsize);

        read(fd,sz1,lsize);
        read(fd,sz2,lsize);

        size_t s1=atol(sz1);
        size_t s2=atol(sz2);

        read(fd,input1,s1);
        read(fd,input2,s2);

        std::string in1,in2;
        in1.assign(input1);
        in2.assign(input2);

        std::cout<<in1<<std::endl;
        std::cout<<in2<<std::endl;

        write(fd,output.c_str(),output.size());
        write(fd,nullptr,0);
    }
    
    return 0;
}