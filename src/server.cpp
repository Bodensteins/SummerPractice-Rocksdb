#include "include/remote_compaction_server.h"

#include <stdlib.h>

int portnum=20000;

int main(int ac, char *av[]){
    if(ac==2){
        portnum=atoi(av[1]);
    }

    RemoteCompactionServer server(portnum);
    server.run();
    return 0;
}
