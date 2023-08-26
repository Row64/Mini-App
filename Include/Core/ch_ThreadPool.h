#ifndef CH_THREADPOOL_H
#define CH_THREADPOOL_H

////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Jakob Progsch, Václav Zeman
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source
//    distribution.
////////////////////////////////////////////////////////////////////////////////

// Basic threadpool code adapted from: https://github.com/progschj/ThreadPool
// Modifications Copyright (C) 2021 Row64

#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

#include <exception>
#include <stdexcept>

#include "ch_AppMessage.h"

typedef AppCore::AppMessage msg_t;

#define MAX_THREADS (std::thread::hardware_concurrency() - 1)

static std::exception_ptr globalExceptionPtr = nullptr;

class ch_ThreadPool {

public:

    ch_ThreadPool( uint8_t inCount = 0 );
    ~ch_ThreadPool();

    ch_ThreadPool(const ch_ThreadPool&) = delete;
    ch_ThreadPool &operator=(const ch_ThreadPool&) = delete;

    uint8_t     GetNbThreads();
    uint8_t     GetNbThreadsFree();
    void        JoinThreads();     
    void        DetachThreads();
    
    void        AddThreads( uint8_t inCount );

    void        AddMsg( msg_t inMsg );
    msg_t       GetNextMsg();
    int         GetMsgCount();

    void        ExecMainQueue();

    // Push a job to the threadpool. These are executed asynchronously as soon as a thread becomes available.
    template<class F, class... Args>
    auto PushJob(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
        using return_type = typename std::invoke_result_t<F, Args...>;
        auto task = std::make_shared< std::packaged_task<return_type()> >(
                        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );
            
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(QueueMutex);     // don't allow enqueueing after stopping the pool
            if(Done)
                throw std::runtime_error("PushJob on stopped ThreadPool");
            Queue.emplace([task](){ (*task)(); });
        }
        QueueCondition.notify_one();
        return res;
    };

    // Push a job to a dedicated secondary thread. These are executed one after the other in order of submission.
    template<class F, class... Args>
    auto PushOrderedJob(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared< std::packaged_task<return_type()> >(
                        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );
            
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(OrderedMutex);     // don't allow enqueueing after stopping the pool
            if(Done)
                throw std::runtime_error("PushOrderedJob on stopped ThreadPool");
            OrderedQueue.emplace([task](){ (*task)(); });
        }
        OrderedCondition.notify_one();
        return res;
    };

    // Push a job to be executed at the start of the next frame. All jobs in the queue are executed on the MAIN thread
    // at the start of each frame. 
    template<class F, class... Args>
    auto PushMainJob(F&& f, Args&&... args) -> void {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared< std::packaged_task<return_type()> >(
                        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );

        MainQueue.emplace([task](){ (*task)(); });
    };


private:

    uint8_t                             NbThreads;
    uint8_t                             NbThreadsFree;
    std::mutex                          NbThreadsMutex;
    std::vector<std::thread>            Threads;

    std::queue<std::function<void()>>   Queue;
    std::condition_variable             QueueCondition;
    std::mutex                          QueueMutex;
    bool                                Done;

    std::queue<msg_t>                   MsgQueue;
    std::mutex                          MsgQueueMutex;

    std::queue<std::function<void()>>   OrderedQueue;   // Jobs in this queue run in order of submission
    std::condition_variable             OrderedCondition;
    std::mutex                          OrderedMutex;
    std::thread                         OrderedThread;  // Dedicated secondary thread for running OrderedQueue jobs

    std::queue<std::function<void()>>   MainQueue;

    void    RunThread();
    void    RunOrderedThread();

};

#ifndef TPOOL
    extern ch_ThreadPool* TPOOL;
#endif

void SetTPOOL( ch_ThreadPool* inThreadPool );

#endif /* CH_THREADPOOL_H */
