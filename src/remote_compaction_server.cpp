#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <cassert>

#include "include/socket_lib.h"
#include "include/remote_compaction_server.h"


RemoteCompactionServer::RemoteCompactionServer(int port)
    :   portnum(port),cur_isz(BUFSIZ/2){

    input=(char*)malloc(cur_isz);
    assert(input!=nullptr);

    init_options();
}

RemoteCompactionServer::~RemoteCompactionServer(){
    if(input!=nullptr){
        free(input);
    }
}

void RemoteCompactionServer::run(){
    sock_id=make_server_socket_q(portnum,16);
    accept_request(sock_id);
}

void RemoteCompactionServer::init_options(){
    options_override.env = options.env;
    options_override.file_checksum_gen_factory =
        options.file_checksum_gen_factory;
    options_override.comparator = options.comparator;
    options_override.merge_operator = options.merge_operator;
    options_override.compaction_filter = options.compaction_filter;
    options_override.compaction_filter_factory =
        options.compaction_filter_factory;
    options_override.prefix_extractor = options.prefix_extractor;
    options_override.table_factory = options.table_factory;
    options_override.sst_partitioner_factory = options.sst_partitioner_factory;
}

rocksdb::Status RemoteCompactionServer::do_compact(){
    std::string input_;
    for(int i=0;i<isz;i++){
        input_.push_back(input[i]);
    }
    std::string path_(path);
    std::string tmp_path_=path_+"/cmpctsrv_tmp";
    output.clear();
    fprintf(stdout,"into OpenAndCompact\n");
    return rocksdb::DB::OpenAndCompact(path_, tmp_path_, input_, &output,options_override);
}

void RemoteCompactionServer::accept_request(int sock_id){
    int fd;
    while(true){
        fd=accept(sock_id,nullptr,nullptr);
        memset(path_sz,0,lsize);
        memset(input_sz,0,lsize);

        if(read(fd,path_sz,lsize)!=lsize){
            fprintf(stderr,"read path size\n");
            continue;
        }
        if(read(fd,input_sz,lsize)!=lsize){
            fprintf(stderr,"read input size\n");
            continue;
        }

        psz=atol(path_sz);
        isz=atol(input_sz);
        //fprintf(stdout,"input size:%lu\n",isz);

        //accept meta data and input
        memset(path,0,sizeof(path));
        int n=read(fd,path,psz);
        if(n!=psz){
            fprintf(stderr,"read path\n");
            continue;
        }
        //fprintf(stdout,"path:%s\n",path);
        
        if(cur_isz<=isz){
            cur_isz=2*isz;
            input=(char*)realloc(input,cur_isz);
        }
        memset(input,0,BUFSIZ);
        n=read(fd,input,isz);
        if(n!=isz){
            fprintf(stderr,"read input\n");
            continue;
        }

        //call rocksdb::DB::OpenAndCompact
        rocksdb::Status s=do_compact();
        if(!s.ok()){
            fprintf(stderr,"OpenAndCompact error:%s\n",s.getState());
            continue;
        }

        fprintf(stdout,"send output back\n");
        
        //send output back
        memset(output_sz,0,lsize);
        sprintf(output_sz,"%lu",output.size());

        if(write(fd,output_sz,lsize)!=lsize){
            fprintf(stderr,"write output size\n");
            continue;
        }

        n=write(fd,output.c_str(),output.size());
        if(n!=output.size()){
            fprintf(stderr,"write output\n");
            continue;
        }
        
    }
}
