#include "include/socket_lib.h"
#include "include/thread_pool.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

#include <string>
#include <cassert>

class ProxyTask{
public:
    ProxyTask()=default;
    
    ProxyTask(const int fd_, const int portnum_);

    ProxyTask(const ProxyTask &task){
        *this=task;
    }

    ProxyTask(ProxyTask &&task){
        *this=task;
    }

    ~ProxyTask();

    void run();

    ProxyTask& operator= (const ProxyTask &task);

    ProxyTask& operator= (ProxyTask &&task);

private:
    int receive_input_from_client();
    int send_input_to_server();
    int receive_output_from_server();
    int send_output_to_client();

    const static size_t lsize=11;

    const char *hostname="bodensteins";

    int fd_client;
    int portnum;
    int fd_server;
    
    char path_sz[lsize];
    char input_sz[lsize];
    char output_sz[lsize];

    size_t psz=0;
    size_t isz=0;
    size_t osz=0;

    size_t cur_insize=0;
    size_t cur_outsize=0;

    char path[256];
    char *input=nullptr;
    char *output=nullptr;
};


ProxyTask::ProxyTask(const int fd_, const int portnum_)
    :   fd_client(fd_), portnum(portnum_),cur_insize(BUFSIZ),cur_outsize(BUFSIZ/2){
    input=(char*)malloc(cur_insize);
    assert(input!=nullptr);
    output=(char*)malloc(cur_outsize);
    assert(output!=nullptr);
}

ProxyTask::~ProxyTask(){
    if(input!=nullptr){
        free(input);
    }
    if(output!=nullptr){
        free(output);
    }
}

void ProxyTask::run(){
    if(receive_input_from_client()==-1){
        fprintf(stderr,"receive_input_from_client\n");
    }

    if(send_input_to_server()==-1){
        fprintf(stderr,"send_input_to_server\n");
    }
    
    if(receive_output_from_server()==-1){
        fprintf(stderr,"receive_output_from_server\n");
    }
    
    if(send_output_to_client()==-1){
        fprintf(stderr,"send_output_to_client\n");
    }
}

ProxyTask& ProxyTask::operator= (const ProxyTask &task){
    if(this==&task)
        return *this;

    fd_client=task.fd_client;
    portnum=task.portnum;
    fd_server=task.fd_server;

    memcpy(path_sz,task.path_sz,lsize);
    memcpy(input_sz,task.path_sz,lsize);
    memcpy(output_sz,task.path_sz,lsize);

    psz=task.psz;
    isz=task.isz;
    osz=task.osz;

    if(psz>0){
        memcpy(path,task.path,psz);
    }


    if(task.input!=nullptr){
        if(input!=nullptr){
            if(cur_insize!=task.cur_insize)
                input=(char*)realloc(input,task.cur_insize);
        }
        else{
            input=(char*)malloc(task.cur_insize);
        }
        assert(input!=nullptr);

        cur_insize=task.cur_insize;

        if(isz>0)
            memcpy(input,task.input,isz);
    }
    else{
        if(input!=nullptr){
            free(input);
            input=nullptr;
        }
    }


    if(task.output!=nullptr){
        if(output!=nullptr){
            if(cur_outsize!=task.cur_outsize)
                output=(char*)realloc(output,task.cur_outsize);
        }
        else{
            output=(char*)malloc(task.cur_outsize);
        }
        assert(output!=nullptr);

        cur_outsize=task.cur_outsize;
        
        if(osz>0)
            memcpy(output,task.output,osz);
    }
    else{
        if(output!=nullptr){
            free(output);
            output=nullptr;
        }
    }

    return *this;
}

ProxyTask& ProxyTask::operator= (ProxyTask &&task){
    fd_client=task.fd_client;
    portnum=task.portnum;
    fd_server=task.fd_server;

    memcpy(path_sz,task.path_sz,lsize);
    memcpy(input_sz,task.path_sz,lsize);
    memcpy(output_sz,task.path_sz,lsize);

    psz=task.psz;
    isz=task.isz;
    osz=task.osz;

    if(psz>0){
        memcpy(path,task.path,psz);
    }

    if(input!=nullptr){
        free(input);
    }
    if(output!=nullptr){
        free(output);
    }

    input=task.input;
    output=task.output;

    cur_insize=task.cur_insize;
    cur_outsize=task.cur_outsize;

    task.input=nullptr;
    task.output=nullptr;

    return *this;
}

int ProxyTask::receive_input_from_client(){
    memset(path_sz,0,lsize);
    memset(input_sz,0,lsize);

    if(read(fd_client,path_sz,lsize)!=lsize){
        fprintf(stderr,"read path size\n");
        return -1;
    }
    if(read(fd_client,input_sz,lsize)!=lsize){
        fprintf(stderr,"read input size\n");
        return -1;
    }

    psz=atol(path_sz);
    isz=atol(input_sz);
    //fprintf(stdout,"input size:%lu\n",isz);

    //accept path and input
    memset(path,0,sizeof(path));
    int n=read(fd_client,path,psz);
    if(n!=psz){
        fprintf(stderr,"read path\n");
        return -1;
    }
    //fprintf(stdout,"path:%s\n",path);
    
    if(cur_insize<=isz){
        cur_insize=2*isz;
        input=(char*)realloc(input,cur_insize);
    }
    memset(input,0,BUFSIZ);
    n=read(fd_client,input,isz);
    if(n!=isz){
        fprintf(stderr,"read input\n");
        return -1;
    }
    return 0;
}

int ProxyTask::send_input_to_server(){
    fd_server=connect_to_server(hostname,portnum);

    if(write(fd_server,path_sz,lsize)!=lsize){
        fprintf(stderr,"send path size fail\n");
        return -1;
    }

    if(write(fd_server,input_sz,lsize)!=lsize){
        fprintf(stderr,"send input size fail\n");
        return -1;
    }

    int n=write(fd_server,path,psz);
    if(n!=psz){
        fprintf(stderr,"send path fail\n");
        return -1;
    }

    n=write(fd_server,input,isz);
    if(n!=isz){
        fprintf(stderr,"send input fail\n");
        return -1;
    }

    return 0;
}

int ProxyTask::receive_output_from_server(){
    memset(output_sz,0,lsize);

    if(read(fd_server,output_sz,lsize)!=lsize){
        fprintf(stderr,"receive output size fail\n");
        return -1;
    }

    osz=atol(output_sz);
    //fprintf(stdout,"output size:%lu\n",osz);

    if(cur_outsize<=osz){
        fprintf(stderr,"yes\n");
        cur_outsize=2*osz;
        output=(char*)realloc(output,cur_outsize);
        assert(output!=nullptr);
    }
    memset(output,0,osz);

    int n=read(fd_server,output,osz);
    if(n<=0){
        fprintf(stderr,"receive output fail, %d\n",n);
        return -1;
    }

    close(fd_server);
    return 0;
}

int ProxyTask::send_output_to_client(){
    memset(output_sz,0,lsize);
    sprintf(output_sz,"%lu",osz);

    if(write(fd_client,output_sz,lsize)!=lsize){
        fprintf(stderr,"write output size\n");
        return -1;
    }

    int n=write(fd_client,output,osz);
    if(n!=osz){
        fprintf(stderr,"write output\n");
        return -1;
    }
    return 0;
}


int portnum=10000;
int realport1=20000, realport2=30000;
int threadnum=25;
int whitch=0;

ThreadPool<ProxyTask> *tp;
std::vector<int> ports;

void accept_request(int sock_id){
    int fd;
    size_t ports_size=ports.size();
    while (true){
        fd=accept(sock_id,nullptr,nullptr);

        int port=ports[whitch];
        whitch=(whitch+1)%ports_size;

        ProxyTask task(fd,port);
        tp->pushBackTask(task);
    }
    
}

int main(int ac, char *av[]){
    if(ac>1){
        for(int i=1;i<ac;i++){
            ports.push_back(atoi(av[i]));
        }
    }else{
        ports.push_back(20000);
    }

    tp=new ThreadPool<ProxyTask>(threadnum);
    
    int sid=make_server_socket_q(portnum,32);
    if(sid==-1)
        err(1,"make socket");

    accept_request(sid);
    
    delete tp;

    return 0;
}
