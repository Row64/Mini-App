#ifndef CH_THREADPOOL_CPP
#define CH_THREADPOOL_CPP

#include "ch_ThreadPool.h"


ch_ThreadPool::ch_ThreadPool( uint8_t inCount ) :
    Done( false ) {
    NbThreads = ( inCount > MAX_THREADS ) ? MAX_THREADS : inCount;
    NbThreadsFree = NbThreads;
    for (int i = 0; i < (int) NbThreads; i++ ) {
        Threads.push_back( std::thread( &ch_ThreadPool::RunThread, this ) );
        std::cout << "Created Thread " << i << std::endl;
    }
    // We set up one extra thread specifically for running ordered queue submissions.
    // However, the NbThreadsFree is still tracked based on hardware concurrency.
    OrderedThread = std::thread( &ch_ThreadPool::RunOrderedThread, this );
    std::cout << "Created Ordered Thread" << std::endl;
}

ch_ThreadPool::~ch_ThreadPool() {
    JoinThreads();
}

uint8_t ch_ThreadPool::GetNbThreads() {
    return NbThreads;
}

uint8_t ch_ThreadPool::GetNbThreadsFree() {
    return NbThreadsFree;
}

void ch_ThreadPool::JoinThreads() {
    {
        std::scoped_lock<std::mutex, std::mutex> mlock( QueueMutex, OrderedMutex );
        Done = true;
    }
    QueueCondition.notify_all();
    OrderedCondition.notify_all();
    for (int i = 0; i < Threads.size(); i++) {
        if ( Threads[i].joinable() ) {
            Threads[i].join();
            std::cout << "Joined Thread " << i << std::endl;
        }
    }
    if ( OrderedThread.joinable() ) {
        OrderedThread.join();
        std::cout << "Joined OrderedThread" << std::endl;
    }
}

void ch_ThreadPool::DetachThreads() {
    for (int i = 0; i < Threads.size(); i++) {
        if ( Threads[i].joinable() ) {
            Threads[i].detach();
            std::cout << "Detached Thread " << i << std::endl;
        }
    }
}

void ch_ThreadPool::AddThreads( uint8_t inCount ) {
    for (int i = 0; i < (int) inCount; i++ ) {
        Threads.push_back( std::thread( &ch_ThreadPool::RunThread, this ) );
        std::cout << "Created Thread " << i << std::endl;
        { 
            std::unique_lock<std::mutex> mlock( NbThreadsMutex );
            NbThreads++;
            NbThreadsFree++; 
        }
    }
}

void ch_ThreadPool::AddMsg( msg_t inMsg ) {
    std::lock_guard<std::mutex> mlock( MsgQueueMutex );
    MsgQueue.push( inMsg );
}

msg_t ch_ThreadPool::GetNextMsg() {
    msg_t msg;
    {
        std::lock_guard<std::mutex> mlock( MsgQueueMutex );
        msg = MsgQueue.front();
        MsgQueue.pop();
    }
    return msg;
}

int ch_ThreadPool::GetMsgCount() {
    return (int) MsgQueue.size();
}

void ch_ThreadPool::ExecMainQueue() {
    std::function<void()> task;
    while ( !MainQueue.empty() ) {
        task = std::move( MainQueue.front() );
        MainQueue.pop();
        task();
    }
}

void ch_ThreadPool::RunThread() {
    
    while ( !Done ) {

        std::function<void()> task;

        {   // Mutex locked within this scope
            std::unique_lock<std::mutex> mlock( QueueMutex );
            QueueCondition.wait( mlock, 
                [&]{ return ( Queue.size() > 0 ) || Done; }
            );
            if (Done || Queue.size() == 0 ) { return; }
            task = std::move( Queue.front() );
            Queue.pop();   
        }

        { std::unique_lock<std::mutex> mlock( NbThreadsMutex ); NbThreadsFree--; }
        task();
        { std::unique_lock<std::mutex> mlock( NbThreadsMutex ); NbThreadsFree++; }
    }
    
}

void ch_ThreadPool::RunOrderedThread() {
    
    while ( !Done ) {

        std::function<void()> task;

        {   // Mutex locked within this scope
            std::unique_lock<std::mutex> mlock( OrderedMutex );
            OrderedCondition.wait( mlock, 
                [&]{ return ( OrderedQueue.size() > 0 ) || Done; }
            );
            if (Done || OrderedQueue.size() == 0 ) { return; }
            task = std::move( OrderedQueue.front() );
            OrderedQueue.pop();   
        }

        { std::unique_lock<std::mutex> mlock( NbThreadsMutex ); NbThreadsFree--; }
        task();
        { std::unique_lock<std::mutex> mlock( NbThreadsMutex ); NbThreadsFree++; }
    }
    
}

#ifndef TPOOL
    ch_ThreadPool* TPOOL = NULL;
#endif
void SetTPOOL( ch_ThreadPool* inThreadPool ) {    // This is called in main.cpp
    TPOOL = inThreadPool;
};

#endif /* CH_THREADPOOL_CPP */
