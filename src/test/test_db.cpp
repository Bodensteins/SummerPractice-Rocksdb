#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/write_batch.h>
#include <iostream>

using namespace std;
using namespace rocksdb;

string path1="./db/data1";
string path2="./db/data2";

int main(){
    DB* db1;
    Options options1;
    options1.disable_auto_compactions=true;
    options1.create_if_missing=true;
    Status status1=DB::Open(options1,path1,&db1);
    assert(status1.ok());

    DB* db2;
    Options options2;
    options2.disable_auto_compactions=true;
    options2.create_if_missing=true;
    Status status2=DB::Open(options2,path2,&db2);
    assert(status2.ok());
   
   Iterator *iter1=db1->NewIterator(ReadOptions());
   iter1->SeekToFirst();

   Iterator *iter2=db2->NewIterator(ReadOptions());
   iter2->SeekToFirst();

    while(iter1->Valid() && iter2->Valid()){
        cout<<iter1->key().ToString()<<" "<<iter1->value().ToString()<<endl;
        cout<<iter2->key().ToString()<<" "<<iter2->value().ToString()<<endl;
        iter1->Next();
        iter2->Next();
    }

    return 0;
}
