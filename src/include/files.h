#ifndef _FILES_H_
#define _FILES_H_

#include <string>

int copy_all_files_in_dir(const std::string &src_dir, const std::string &dst_dir);

int delete_all_files_in_dir(const std::string &dir);

#endif