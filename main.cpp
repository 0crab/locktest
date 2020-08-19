#include <iostream>
#include <thread>
#include <vector>
#include "rwlock.h"

#define WRITE_THREAD_NUM  4
#define READ_THREAD_NUM  4

#define TEST_NUM 1000000

using namespace std;

spinlock lock;
size_t opnum;

void write_thread(int tid);
void read_thread(int tid);

int main() {
    opnum = 0;
    vector<thread> writers,readers;
    for(int i = 0; i < WRITE_THREAD_NUM; i++){
        writers.push_back(thread(write_thread,i));
    }

    for(int i = 0; i < READ_THREAD_NUM; i++){
        readers.push_back(thread(read_thread,i));
    }

    for(int i = 0; i < WRITE_THREAD_NUM; i++){
        writers[i].join();
    }

    for(int i = 0; i < READ_THREAD_NUM; i++){
        readers[i].join();
    }

    return 0;
}


void write_thread(int tid){
    for(int i = 0; i < TEST_NUM; i++){
        LockManager lockmanager;
        lockmanager.reset(&lock);
        lockmanager->lock();
   //     printf("wlock: %lu\n",lockmanager->rwlock);
        size_t a = opnum;
        size_t b;
        if(tid % 2 ){
            b = --opnum;
            assert(b - a == -1);
        }else{
            b = ++opnum;
            assert(b - a == 1);
        }
    }
}

void read_thread(int tid){
    for(int i = 0; i < TEST_NUM; i++){
        while(true){
            LockManager lockmanager;
            lockmanager.reset(&lock);
            lockmanager->lock(true);
         //   printf("--------------------------rlock %lu\n",lockmanager->rwlock);
            size_t a = opnum;
            assert(a == opnum);

            if(lockmanager->try_upgradeLock()){
                assert(lockmanager->isWriteLocked());
                assert(a == opnum);
                size_t b;
                if(tid % 2 ){
                    b = ++opnum;
                    assert(b - a == 1);
                }else{
                    b = --opnum;
                    assert(b - a == -1);
                }
                break;
            }
        }
    }
}
