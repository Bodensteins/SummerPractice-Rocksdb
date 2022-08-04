#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/write_batch.h>

using namespace std;
using namespace rocksdb;

string path1="./db/data1";
string path2="./db/data2";

int main(){
    DB* db1;
    Options options1;
    options1.create_if_missing=true;
    Status status1=DB::Open(options1,path1,&db1);
    assert(status1.ok());
   
    WriteBatch batch1;

    for(int i=0;i<100000;i++){
        batch1.Put(to_string(i),to_string(2*i+1));
    }
    db1->Write(WriteOptions(),&batch1);

    DB* db2;
    Options options2;
    options2.create_if_missing=true;
    Status status2=DB::Open(options2,path2,&db2);
    assert(status2.ok());
   
    WriteBatch batch2;

    for(int i=0;i<100000;i++){
        batch2.Put(to_string(i),to_string(2*i+1));
    }
    db2->Write(WriteOptions(),&batch2);

    return 0;
}
