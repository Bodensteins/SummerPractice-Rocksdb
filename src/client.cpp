#include "include/compaction_service_impl.h"

using namespace std;
using namespace rocksdb;

string path="./db/data1";
string path_dup="./db/data2";

const int portnum=10000;

int main(){
    DB* db;
    Options options;
    options.create_if_missing=true;
    options.disable_auto_compactions=true;
    options.compaction_service=make_shared<CompactionServiceImpl>(path,portnum,path_dup,options);
    Status s=DB::Open(options,path,&db);
    if(!s.ok()){
        fprintf(stderr,"%s\n",s.getState());
    }
    assert(s.ok());

    string file1=path+"/000028.sst";
    string file2=path+"/000031.sst";
    string file3=path+"/000036.sst";

    string file4=path+"/000041.sst";
    string file5=path+"/000046.sst";

    string file6=path+"/000047.sst";

    db->CompactFiles(CompactionOptions(),{file1,file2,file3},2);
    db->CompactFiles(CompactionOptions(),{file4,file5},3);
    db->CompactFiles(CompactionOptions(),{file6},4);

    return 0;
}