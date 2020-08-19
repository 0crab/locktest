#include <assert.h>
#include <memory>

class spinlock {
public:

spinlock() :  is_migrated_(true),write_unlock(false) { rwlock = 0; }

spinlock(const spinlock &other) noexcept
        : is_migrated_(other.is_migrated()) {
    rwlock = 0;
}

spinlock &operator=(const spinlock &other) noexcept {
    is_migrated() = other.is_migrated();
    return *this;
}

inline bool isWriteLocked() {
    return rwlock & 1;
}


inline bool isReadLocked() {
    return rwlock & ~1;
}

inline bool isLocked() {
    return rwlock;
}

void lock() noexcept {
    //printf("%lu try lock write %lu:%lld\n",pthread_self()%1000,(uint64_t)(&rwlock)%1000,rwlock);
    while (1) {
        long long v = rwlock;
        if((v & 1)){
            int a = 1;
        }
        if (__sync_bool_compare_and_swap(&rwlock, v & ~1, v | 1)) {

            while (v & ~1) { // while there are still readers
                v = rwlock;
            }
            write_unlock = true;
            return;
        }
    }
}

void lock(bool r){
    //printf("--------------------------%lu try lock read %lu:%lld\n",pthread_self()%1000,(uint64_t)(&rwlock)%1000,rwlock);
    while (1) {
        while (rwlock & 1) {}
        if ((__sync_add_and_fetch(&rwlock, 2) & 1) == 0) return; // when we tentatively read-locked, there was no writer
        __sync_add_and_fetch(&rwlock, -2); // release our tentative read-lock
    }
}

inline bool try_upgradeLock() {
    long long v = rwlock;
    if (__sync_bool_compare_and_swap(&rwlock, v & ~1, v | 1)) {
        __sync_add_and_fetch(&rwlock, -2);
        while (v & ~1) { // while there are still readers
            v = rwlock;
        }
        write_unlock = true;
        return true;
    }
    return false;
}

inline void degradeLock(){
    write_unlock = false;
    __sync_add_and_fetch(&rwlock,1);
}


void unlock() noexcept {
    if(isWriteLocked() && write_unlock){
       // printf("%lu write unlock %lu:%lld\n",pthread_self()%1000,(uint64_t)(&rwlock)%1000,rwlock);
        write_unlock = false;
        __sync_add_and_fetch(&rwlock, -1);
    }else {
        assert(rwlock != 1);
       // printf("--------------------------%lu read unlock %lu:%lld\n",pthread_self()%1000,(uint64_t)(&rwlock)%1000,rwlock);
        __sync_add_and_fetch(&rwlock, -2);
    }
}

    void unlock1() noexcept {
            assert(rwlock & 1);
            __sync_add_and_fetch(&rwlock, -1);
    }

    void unlock1(bool r) noexcept {
        assert(rwlock !=1);
        __sync_add_and_fetch(&rwlock, -2);
    }

bool try_lock() noexcept {
    assert(rwlock&1);
    return !(rwlock & 1);
}


bool &is_migrated() noexcept { return is_migrated_; }
bool is_migrated() const noexcept { return is_migrated_; }

    volatile long long rwlock;
private:

volatile bool write_unlock;
//std::atomic_flag lock_;
bool is_migrated_;
};

struct LockDeleter {
    void operator()(spinlock *l) const { l->unlock(); }
};

using LockManager = std::unique_ptr<spinlock, LockDeleter>;