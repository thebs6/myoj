#include "CurrentThread.h"

namespace CurrentThread
{

    __thread int t_cachedTid = 0;

    void cacheTid()
    {
        //没有缓存则系统调用
        if(t_cachedTid == 0)
        {
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}