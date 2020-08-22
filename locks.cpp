#include "tracer.h"
#include <thread>
#include <vector>
#include <iostream>
#include "locks_def.h"

#define CONFLICT true

#define TEST_NUM 100000

#define THREAD_NUM 1

using namespace std;

#if(CONFLICT)
Lock mlock;
#endif

size_t a = 0 ;

unsigned long * runtimelist;

void show_info();

void worker(int tid){
#if(!CONFLICT)
    Lock mlock;
#endif
    Tracer t;
    t.startTime();
    std::atomic<uint64_t>  lock_{0};
    for(int i = 0 ; i < TEST_NUM; i++){
        mlock.lock();
        a++;
        mlock.unlock();
    }
    runtimelist[tid] += t.getRunTime();
}

int main(){
    show_info();
    runtimelist = new unsigned long [THREAD_NUM]();

    vector<thread> workers;
    for(int i = 0; i < THREAD_NUM; i++){
        workers.push_back(thread(worker,i));
    }
    for(int i = 0; i < THREAD_NUM; i++){
        workers[i].join();
    }
    unsigned long runtime=0;
    for(int i = 0;i < THREAD_NUM; i++){
        runtime += runtimelist[i];
    }
    runtime /= (THREAD_NUM);
    printf("runtime:%lu\n",runtime);
}

void show_info(){

#ifdef TEST_AND_SET
    cout<<"TEST_AND_SET"<<endl;
#endif

#ifdef COMPARE_AND_SWAP
    cout<<"COMPARE_AND_SWAP"<<endl;
#endif

#ifdef COMPARE_EXCHANGE
    cout<<"COMPARE_EXCHANGE"<<endl;
#endif

#ifdef MUTEX
    cout<<"MUTEX"<<endl;
#endif

#ifdef SPINLOCK
    cout<<"SPINLOCK"<<endl;
#endif
    cout<<"CONFLICT:"<<CONFLICT<<endl;
    cout<<"YIELD:"<<YIELD<<endl;
    cout<<"TEST_NUM:"<<TEST_NUM<<endl;

}

