#include "tracer.h"
#include <atomic>
#include <thread>
#include <vector>

#define TEST_NUM 100000

#define THREAD_NUM 4

#define SPINLOCK true

using namespace std;

class Rwlock{
public:
    Rwlock():rwlock(0){}
    void lock(){
        while (1) {
            long long v = rwlock;
            //CAS will fail is other thread sets read lock between these two statement
            if (__sync_bool_compare_and_swap(&rwlock, v & ~1, v | 1)) {
                while (v & ~1) { // while there are still readers
                    v = rwlock;
                }
                return;
            }
        }
    }

    void unlock(){
        __sync_add_and_fetch(&rwlock, -1);
    }

private:
    volatile long long rwlock;
};

class Spinlock{
public:
    Spinlock() { lock_.clear(); }
    void lock() noexcept {
        while (lock_.test_and_set(std::memory_order_acq_rel))
            ;
    }

    void unlock() noexcept { lock_.clear(std::memory_order_release); }

private:
    atomic_flag lock_;

};
#if(SPINLOCK)
    Spinlock lock;
#else
    Rwlock lock;
#endif

size_t a = 0 ;

unsigned long * runtimelist;

void worker(int tid){
    Tracer t;
    t.startTime();
    for(int i = 0 ; i < TEST_NUM; i++){
        lock.lock();
        a++;
        lock.unlock();
    }
    runtimelist[tid] += t.getRunTime();
}

int main(){
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

