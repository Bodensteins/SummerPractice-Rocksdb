#include "include/remote_compaction_server.h"

const int portnum=30000;

int main(int ac, char *av[]){
    RemoteCompactionServer server(portnum);
    server.run();
    return 0;
}
