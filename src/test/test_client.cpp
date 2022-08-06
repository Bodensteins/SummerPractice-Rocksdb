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

#include <string>

#include "../include/socket_lib.h"



int portnum=32000;
int sock_id;
int lsize=11;

std::string input1("test1 why do you do this");
std::string input2("test2 stryyyyy yes ma nrii");

//const char* pathid="path 9";
char sz1[11];
char sz2[11];

char output[BUFSIZ];

int main(){
    sock_id=connect_to_server("bodensteins",portnum);

    sprintf(sz1,"%lu",input1.size());
    sprintf(sz2,"%lu",input2.size());

    write(sock_id,sz1,lsize);
    write(sock_id,sz2,lsize);

    write(sock_id,input1.c_str(),input1.size());
    write(sock_id,input2.c_str(),input2.size());

    write(sock_id,nullptr,0);

    memset(output,0,BUFSIZ);
    read(sock_id,output,BUFSIZ);
    std::string out("test");
    out.assign(output);
    std::cout<<out<<std::endl;
    close(sock_id);
    return 0;
}
