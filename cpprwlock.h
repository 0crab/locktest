#include <assert.h>
#include <memory>
#include <atomic>
using counter_type = int64_t;
class spinlock {
public:

    spinlock() : elem_counter_(0),is_migrated_(true),write_unlock(false),lock_(0) {  }

    spinlock(const spinlock &other) noexcept
            :elem_counter_(other.elem_counter()),
            is_migrated_(other.is_migrated()),
            lock_(0) {
    }

    spinlock &operator=(const spinlock &other) noexcept {
        elem_counter() = other.elem_counter();
        is_migrated() = other.is_migrated();
        return *this;
    }

    inline bool isWriteLocked() {
        return lock_.load() & 1;
    }


    inline bool isReadLocked() {
        return lock_.load() & ~1;
    }

    inline bool isLocked() {
        return lock_.load();
    }

    void lock() noexcept {
        //printf("%lu try lock write %lu:%lld\n",pthread_self()%1000,(uint64_t)(&rwlock)%1000,rwlock);
        while (1) {
            uint64_t expected = lock_.load() & ~1;
            if(lock_.compare_exchange_strong(expected , expected | 1)){
                while(expected){
                    expected = lock_.load() & ~1; //an extra judgment
                }
                write_unlock = true;
                return;
            }
        }
    }

    void lock(bool r){
        //printf("--------------------------%lu try lock read %lu:%lld\n",pthread_self()%1000,(uint64_t)(&rwlock)%1000,rwlock);
        while (1) {
            while (lock_.load() & 1) {}
            if ((lock_.fetch_add(2) & 1) == 0) return; // when we tentatively read-locked, there was no writer
            lock_.fetch_add(-2); // release our tentative read-lock
        }
    }

    inline bool try_upgradeLock() {
        uint64_t expected = lock_.load() & ~1;
        if (lock_.compare_exchange_strong(expected,expected | 1)) {
            lock_.fetch_add(-2);
            while (expected) { // while there are still readers
                expected = lock_.load() & ~1;
            }
            write_unlock = true;
            return true;
        }
        return false;
    }

    inline void degradeLock(){
        write_unlock = false;
        lock_.fetch_add(1);
    }


    void unlock() noexcept {
        if(isWriteLocked() && write_unlock){
            // printf("%lu write unlock %lu:%lld\n",pthread_self()%1000,(uint64_t)(&rwlock)%1000,rwlock);
            write_unlock = false;
            lock_.fetch_add(-1);
        }else {
            assert(lock_.load() != 1);
            // printf("--------------------------%lu read unlock %lu:%lld\n",pthread_self()%1000,(uint64_t)(&rwlock)%1000,rwlock);
            lock_.fetch_add(-2);
        }
    }

    bool try_lock() noexcept {
        return !(lock_.load() & 1);
    }

    counter_type &elem_counter() noexcept { return elem_counter_; }
    counter_type elem_counter() const noexcept { return elem_counter_; }

    bool &is_migrated() noexcept { return is_migrated_; }
    bool is_migrated() const noexcept { return is_migrated_; }

    std::atomic<uint64_t>  lock_;
private:
    volatile bool write_unlock;
    counter_type elem_counter_;
    bool is_migrated_;
};

struct LockDeleter {
    void operator()(spinlock *l) const { l->unlock(); }
};

using LockManager = std::unique_ptr<spinlock, LockDeleter>;