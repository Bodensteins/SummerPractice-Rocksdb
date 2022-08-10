#include "include/compaction_service_impl.h"
#include "include/files.h"


//CompactionServiceImpl

CompactionServiceImpl::CompactionServiceImpl(const std::string &db_path, const int port)
    :   db_path_(db_path),portnum(port),db_dup_(nullptr),cur_outsize(BUFSIZ){

    output=(char*)malloc(cur_outsize);
    assert(output!=nullptr);
}

CompactionServiceImpl::CompactionServiceImpl(
    const std::string &db_path, const int port, const std::string &db_path_dup, const rocksdb::Options &options_dup)
    :   db_path_(db_path),portnum(port),db_path_dup_(db_path_dup),options_dup_(options_dup),cur_outsize(BUFSIZ){

    output=(char*)malloc(BUFSIZ);
    assert(output!=nullptr);

    options_dup_.compaction_service=std::make_shared<CompactionServiceDupImpl>(db_path_dup_);
    rocksdb::Status s=rocksdb::DB::Open(options_dup_,db_path_dup_,&db_dup_);
    if(!s.ok()){
        fprintf(stderr,"db_dup_ open error: %s\n",s.getState());
        db_dup_=nullptr;
    }
}

CompactionServiceImpl::~CompactionServiceImpl(){
    if(output!=nullptr){
        free(output);
    }

    if(db_dup_!=nullptr){
        rocksdb::Status s=db_dup_->Close();
        if(!s.ok()){
            fprintf(stderr,"db_dup_ close error: %s\n",s.getState());
        }
    }
}

rocksdb::CompactionServiceJobStatus CompactionServiceImpl::StartV2(
      const rocksdb::CompactionServiceJobInfo& info,
      const std::string& compaction_service_input){
    fprintf(stdout,"master calls startV2\n");
    
    assert(info.db_name==db_path_);

    jobs_.emplace(info.job_id,compaction_service_input);
        
    return rocksdb::CompactionServiceJobStatus::kSuccess;
}

rocksdb::CompactionServiceJobStatus CompactionServiceImpl::WaitForCompleteV2(
      const rocksdb::CompactionServiceJobInfo& info,
      std::string* compaction_service_result){
    fprintf(stdout,"master calls WaitForCompleteV2\n");

    assert(info.db_name==db_path_);
    std::string compaction_service_input;
    
    auto iter=jobs_.find(info.job_id);
    if(iter==jobs_.end()){
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }
    compaction_service_input=std::move(iter->second);
    jobs_.erase(iter);

    memset(input_sz,0,lsize);
    memset(path_sz,0,lsize);

    sprintf(path_sz,"%lu",db_path_.size());
    sprintf(input_sz,"%lu",compaction_service_input.size());

    //send input data
    sock_id=connect_to_server(hostname,portnum);
        
    if(write(sock_id,path_sz,lsize)!=lsize){
        fprintf(stderr,"send db_path size fail\n");
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }

    if(write(sock_id,input_sz,lsize)!=lsize){
        fprintf(stderr,"send input size fail\n");
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }

    int n=write(sock_id,db_path_.c_str(),db_path_.size());
    if(n!=db_path_.size()){
        fprintf(stderr,"send db_path fail\n");
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }

    n=write(sock_id,compaction_service_input.c_str(),compaction_service_input.size());
    if(n!=compaction_service_input.size()){
        fprintf(stderr,"send input fail\n");
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }

    memset(output_sz,0,lsize);

    if(read(sock_id,output_sz,lsize)!=lsize){
        fprintf(stderr,"receive output size fail\n");
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }

    size_t outsize=atol(output_sz);
    //fprintf(stdout,"output size:%lu\n",outsize);

    if(cur_outsize<=outsize){
        cur_outsize=2*outsize;
        output=(char*)realloc(output,cur_outsize);
        assert(output!=nullptr);
    }
    memset(output,0,outsize);

    n=read(sock_id,output,outsize);
    if(n<=0){
        fprintf(stderr,"receive output fail, %d\n",n);
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }

    close(sock_id);

    for(int i=0;i<n;i++){
        compaction_service_result->push_back(output[i]);
    }

    if(db_dup_!=nullptr){
        trigger_duplication_compact(compaction_service_input, compaction_service_result);
    }

    return rocksdb::CompactionServiceJobStatus::kSuccess;
}

void CompactionServiceImpl::trigger_duplication_compact(
        const std::string &compaction_service_input,
        std::string* compaction_service_result){
    if(copy_all_files_in_dir(db_path_+"/cmpctsrv_tmp",db_path_dup_+"/cmpctsrv_tmp")==-1){
        fprintf(stderr,"copcopy_all_files_in_dir fail\n");
        return;
    }

    rocksdb::CompactionServiceResult result;
    rocksdb::CompactionServiceResult::Read(*compaction_service_result,&result);
    result.output_path.assign(db_path_dup_+"/cmpctsrv_tmp");
    std::string compaction_service_result_dup;
    result.Write(&compaction_service_result_dup);

    std::string result_dup_path=result.output_path+"/result_dup";
    int result_dup_fd=open(result_dup_path.c_str(),O_WRONLY | O_CREAT | O_TRUNC,0644);
    if(result_dup_fd==-1){
        fprintf(stderr,"open result_dup fail\n");
        return;
    }

    if(write(result_dup_fd,compaction_service_result_dup.c_str(),
            compaction_service_result_dup.size())!=compaction_service_result_dup.size()){
        fprintf(stderr,"write result to result_dup fail\n");
        return;
    }

    close(result_dup_fd);
    
    rocksdb::CompactionServiceInput input;
    rocksdb::CompactionServiceInput::Read(compaction_service_input,&input);
    
    std::vector<std::string> sst_files;
    for(auto file : input.input_files){
        sst_files.push_back(db_path_dup_+"/"+file);
    }

    rocksdb::Status s=db_dup_->CompactFiles(rocksdb::CompactionOptions(),sst_files,input.output_level);
    if(!s.ok()){
        fprintf(stderr,"duplication compaction error: %s\n",s.getState());
        return;
    }

}


//CompactionServiceDupImpl

CompactionServiceDupImpl::CompactionServiceDupImpl(const std::string &db_path)
    :   db_path_(db_path),cur_outsize(BUFSIZ/2){

    output=(char*)malloc(cur_outsize);
    assert(output!=nullptr);

    std::string tmp_path=db_path_+"/cmpctsrv_tmp";
    if(access(tmp_path.c_str(),F_OK)==-1){
        if(mkdir(tmp_path.c_str(),0775)==-1){
            fprintf(stderr,"mkdir error: %s\n",tmp_path.c_str());
            return;
        }
    }
}

CompactionServiceDupImpl::~CompactionServiceDupImpl(){
    if(output!=nullptr){
        free(output);
    }
}

rocksdb::CompactionServiceJobStatus CompactionServiceDupImpl::StartV2(
      const rocksdb::CompactionServiceJobInfo& info,
      const std::string& compaction_service_input){
    fprintf(stdout,"dup calls startV2\n");

    assert(info.db_name==db_path_);

    jobs_.emplace(info.job_id,compaction_service_input);
    
    return rocksdb::CompactionServiceJobStatus::kSuccess;
}

rocksdb::CompactionServiceJobStatus CompactionServiceDupImpl::WaitForCompleteV2(
      const rocksdb::CompactionServiceJobInfo& info,
      std::string* compaction_service_result){
    fprintf(stdout,"dup calls WaitForCompleteV2\n");
    
    assert(info.db_name==db_path_);
    //std::string compaction_service_input;
    
    auto iter=jobs_.find(info.job_id);
    if(iter==jobs_.end()){
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }
    //compaction_service_input=std::move(iter->second);
    jobs_.erase(iter);

    std::string result_path=db_path_+"/cmpctsrv_tmp/result_dup";

    int result_fd=open(result_path.c_str(),O_RDONLY);
    if(result_fd==-1){
        fprintf(stderr,"result_dup open error\n");
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }

    struct stat stat;
    if(fstat(result_fd,&stat)==-1){
        fprintf(stderr,"fstat error\n");
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }

    if(cur_outsize<=stat.st_size){
        cur_outsize=2*stat.st_size;
        output=(char*)realloc(output,cur_outsize);
        assert(output!=nullptr);
    }
    memset(output,0,cur_outsize);

    if(read(result_fd,output,stat.st_size)!=stat.st_size){
        fprintf(stderr,"result read error\n");
        return rocksdb::CompactionServiceJobStatus::kFailure;
    }

    for(int i=0;i<stat.st_size;i++){
        compaction_service_result->push_back(output[i]);
    }

    return rocksdb::CompactionServiceJobStatus::kSuccess;
}
