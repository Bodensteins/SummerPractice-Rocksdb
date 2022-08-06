#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <deque>
#include <vector>

template <class T>
class ThreadPool{
public:
    ThreadPool(const size_t num);

    ~ThreadPool();

    bool empty();

    void lock();

    void unlock();

    void wait();

    void wakeUp();

    void pushBackTask(T &task);

    void popFrontTask(T &task);

private:
    static void* doTask(void *args);

    std::deque<T> taskQueue_;
    std::vector<pthread_t> tids_;
    size_t threadNum_;
    pthread_mutex_t lock_;
    pthread_cond_t cond_;

};

#include "../thread_pool.cpp"

#endif
