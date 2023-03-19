/*
 * @Author: yowayimono 3485000346@qq.com
 * @Date: 2023-03-19 13:38:32
 * @LastEditors: yowayimono 3485000346@qq.com
 * @LastEditTime: 2023-03-19 14:24:18
 * @FilePath: /Myserver/server/Threadpool.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H



#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <assert.h>
#include <functional>


class ThreadPool{
public:
    explicit ThreadPool(size_t threadCount=8):pool_(std::make_shared<Pool>()){
        assert(threadCount>0);
        for(size_t i=0;i<threadCount;i++){
            std::thread([pool=pool_]{
                std::unique_lock<std::mutex>locker(pool->mtx);
                while(true){
                    if(!pool->tasks.empty()){
                        auto task=std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }
                    else if(pool->isClosed)break;
                    else pool->cond.wait(locker);
                }
            }).detach();
        }
    }

    ThreadPool()=default;
    ThreadPool(ThreadPool&&)=default;

    ~ThreadPool(){
            if(static_cast<bool>(pool_)){
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed=true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task){
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }

private:
    struct Pool{
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;

};
#endif