#include "include/thread_pool.h"
//#include <stdio.h>

template <class T>
ThreadPool<T>::ThreadPool(const size_t num)
:   threadNum_(num),
    tids_(num){
    
    pthread_mutex_init(&lock_,nullptr);
    pthread_cond_init(&cond_,nullptr);

    for(size_t i=0;i<threadNum_;i++){
        pthread_t tid;
        pthread_attr_t attr_detached;
        pthread_attr_init(&attr_detached);
        pthread_attr_setdetachstate(&attr_detached,PTHREAD_CREATE_DETACHED);
        pthread_create(&tid,&attr_detached,doTask,this);
        tids_[i]=tid;
    }
}

template <class T>
ThreadPool<T>::~ThreadPool(){
    pthread_mutex_destroy(&lock_);
    pthread_cond_destroy(&cond_);

    for(size_t i=0;i<threadNum_;i++){
        pthread_cancel(tids_[i]);
    }
}

template <class T>
inline bool ThreadPool<T>::empty(){
    return taskQueue_.empty();
}

template <class T>
inline void ThreadPool<T>::lock(){
    pthread_mutex_lock(&lock_);
}

template <class T>
inline void ThreadPool<T>::unlock(){
    pthread_mutex_unlock(&lock_);
}

template <class T>
inline void ThreadPool<T>::wait(){
    pthread_cond_wait(&cond_, &lock_);
}

template <class T>
inline void ThreadPool<T>::wakeUp(){
    pthread_cond_signal(&cond_);
}

template <class T>
void ThreadPool<T>::pushBackTask(T &task){
    lock();
    taskQueue_.push_back(task);
    unlock();
    wakeUp();
}

template <class T>
void ThreadPool<T>::popFrontTask(T &task){
    //不需要加锁，因为doTask函数的循环中已经加锁了
    task=std::move(taskQueue_.front());
    taskQueue_.pop_front();
}

template <class T>
void* ThreadPool<T>::doTask(void *args){
    ThreadPool<T> *tp=(ThreadPool<T> *)args;
    while(true){
        tp->lock();
        while(tp->empty())
            tp->wait();
        T task;
        tp->popFrontTask(task);
        tp->unlock();
        task.run();
    }
    return nullptr;
}
