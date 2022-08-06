#ifndef _CMPCTSRV_H_
#define _CMPCTSRV_H_

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <string>

class RemoteCompactionServer{
public:
    RemoteCompactionServer(int port);
    
    ~RemoteCompactionServer();

    void run();

private:
    void init_options();

    void accept_request(int sock_id);
    
    rocksdb::Status do_compact();

    const static size_t lsize=11;
    
    int portnum;
    int sock_id;
    
    char path_sz[lsize];
    char input_sz[lsize];
    char output_sz[lsize];

    size_t psz;
    size_t isz;
    size_t cur_insize;

    char path[256];
    char *input;
    std::string output;

    rocksdb::Options options;
    rocksdb::CompactionServiceOptionsOverride options_override;
    rocksdb::OpenAndCompactOptions options_compact;

};

#endif