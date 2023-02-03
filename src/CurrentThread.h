#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{

    //extern 说明是全局变量， 但是全局变量在每个线程是共享的， 而__thread关键词可以在每个线程都拷贝一份该变量
    extern __thread int t_cachedTid;
    
    void cacheTid();

    inline int tid()
    {
        if(__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }

}