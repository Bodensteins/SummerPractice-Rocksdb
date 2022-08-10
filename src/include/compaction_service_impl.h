#ifndef _MYCMPCTSRV_H_
#define _MYCMPCTSRV_H_

//#include <rocksdb/db.h>
#include "db.h"
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/write_batch.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "socket_lib.h"

class CompactionServiceImpl : public rocksdb::CompactionService {
public:
    CompactionServiceImpl(const std::string &db_path, const int port);

    CompactionServiceImpl(
      const std::string &db_path, const int port, const std::string &db_path_dup, const rocksdb::Options &options_dup);

    ~CompactionServiceImpl();

    static const char* kClassName() { return "MyCompactionService"; }

    const char* Name() const override { return kClassName(); }

    rocksdb::CompactionServiceJobStatus StartV2(
      const rocksdb::CompactionServiceJobInfo& info,
      const std::string& compaction_service_input) override;

    rocksdb::CompactionServiceJobStatus WaitForCompleteV2(
      const rocksdb::CompactionServiceJobInfo& info,
      std::string* compaction_service_result) override;

private:
    void trigger_duplication_compact(
        const std::string &compaction_service_input,
        std::string* compaction_service_result);

    const static size_t lsize=11;

    const char *hostname="bodensteins";
    int portnum;

    std::atomic_int compaction_num_{0};
    const std::string db_path_;
    const std::string db_path_dup_;
    std::map<uint64_t, std::string> jobs_;

    rocksdb::DB *db_dup_;
    rocksdb::Options options_dup_;

    int sock_id;
    char path_sz[lsize];
    char input_sz[lsize];
    char output_sz[lsize];
    char *output;
    size_t cur_outsize;
};


class CompactionServiceDupImpl : public rocksdb::CompactionService {
public:
    CompactionServiceDupImpl(const std::string &db_path);

    ~CompactionServiceDupImpl();

    static const char* kClassName() { return "MyCompactionServiceDup"; }

    const char* Name() const override { return kClassName(); }

    rocksdb::CompactionServiceJobStatus StartV2(
      const rocksdb::CompactionServiceJobInfo& info,
      const std::string& compaction_service_input) override;

    rocksdb::CompactionServiceJobStatus WaitForCompleteV2(
      const rocksdb::CompactionServiceJobInfo& info,
      std::string* compaction_service_result) override;

private:
    std::atomic_int compaction_num_{0};
    const std::string db_path_;
    std::map<uint64_t, std::string> jobs_;

    size_t cur_outsize;
    char *output;
};

#endif