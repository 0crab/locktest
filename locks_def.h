#include "locks_conf.h"
#include <atomic>
#include <mutex>

using namespace std;


#ifdef TEST_AND_SET
class Lock{
public:
    Lock(){lock_.clear();}
    void lock(){
        while (lock_.test_and_set(std::memory_order_acq_rel)) {
#if(YIELD)
            this_thread::yield();
#endif
        }
    }

    void unlock(){
        lock_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag lock_;
};
#endif

#ifdef COMPARE_AND_SWAP
class Lock{
public:
    Lock():rwlock(0){}
    void lock(){
        while (1) {
            long long v = rwlock;
            if (__sync_bool_compare_and_swap(&rwlock, v & ~1, v | 1)) {
                return;
            }
#if(YIELD)
            this_thread::yield();
#endif
        }
    }

    void unlock(){
        __sync_add_and_fetch(&rwlock, -1);
    }

private:
    volatile long long rwlock;
};
#endif

#ifdef COMPARE_EXCHANGE
class Lock{
public:
    Lock():lock_(0){}
    void lock(){
        while (1) {
            uint64_t expected = 0;
            if(lock_.compare_exchange_strong(expected , 1)){
                return;
            }
#if(YIELD)
            tthis_thread::yield();
#endif
        }
    }

    void unlock(){
        lock_.fetch_add(-1);
    }

private:
    std::atomic<uint64_t>  lock_;
};
#endif

#ifdef MUTEX
class Lock{
public:
    Lock(){}
    void lock(){
        mutex_.lock();
    }

    void unlock(){
        mutex_.unlock();
    }

private:
    mutex mutex_;
};
#endif

#ifdef SPINLOCK
class Lock{
public:
    Lock(){
        pthread_spin_init(&lock_,PTHREAD_PROCESS_PRIVATE);
    }
    void lock(){
        pthread_spin_lock(&lock_);
    }

    void unlock(){
        pthread_spin_unlock(&lock_);
    }

private:
    pthread_spinlock_t lock_;
};
#endif