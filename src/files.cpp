#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <string>

int copy_all_files_in_dir(const std::string &src_dir, const std::string &dst_dir){
    DIR *src_dir_ptr, *dst_dir_ptr;
    struct dirent *src_dirtp, *dst_dirtp;
    if((src_dir_ptr=opendir(src_dir.c_str()))==nullptr){
        fprintf(stderr,"cannot open %s\n",src_dir.c_str());
        return -1;
    }
    
    while((src_dirtp=readdir(src_dir_ptr))!=nullptr){
        if( src_dirtp->d_type!=DT_REG)
            continue;
        //printf("%s\n",src_dirtp->d_name);
        
        std::string name(src_dirtp->d_name);
        std::string src_path=src_dir+"/"+name;
        std::string dst_path=dst_dir+"/"+name;

        int fd_in=open(src_path.c_str(),O_RDONLY);
        if(fd_in==-1){
            fprintf(stderr,"cannot open src file\n");
            return -1;
        }

        struct stat stat;
        if(fstat(fd_in,&stat)==-1){
            fprintf(stderr,"fstat error\n");
            return -1;
        }

        int fd_out=open(dst_path.c_str(),O_CREAT | O_WRONLY | O_TRUNC,0644);
        if(fd_out==-1){
            fprintf(stderr,"cannot open dst file\n");
            return -1;
        }

        for(int ret=0, len=stat.st_size;len>0;len-=ret){
            ret=copy_file_range(fd_in,nullptr,fd_out,nullptr,len,0);
            if(ret==-1){
                fprintf(stderr,"copy_file_range error\n");
                return -1;
            }
        }

        close(fd_in);
        close(fd_out);
    }
    
    closedir(src_dir_ptr);
    return 0;
}

int delete_all_files_in_dir(const std::string &dir){
    DIR *dir_ptr;
    struct dirent *direntp;
    if((dir_ptr=opendir(dir.c_str()))==nullptr){
        fprintf(stderr,"cannot open %s\n",dir.c_str());
        return -1;
    }
    
    while((direntp=readdir(dir_ptr))!=nullptr){
        if(direntp->d_type!=DT_REG)
            continue;

        std::string name(direntp->d_name);
        std::string path=dir+"/"+name;

        if(unlink(path.c_str())==-1){
            fprintf(stderr,"unlink error: %s\n",path.c_str());
            return -1;
        }
    }

    closedir(dir_ptr);
    return 0;
}