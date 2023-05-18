#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>


#include "Buffer.h"

const char Buffer::kCRLF[] = "\r\n";

// 从fd中读
ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    // 利用栈上的64k空间
    char extrabuf[65536] = {0};
    struct iovec vec[2];
    const size_t writable = writableBytes();
    // 用iovec同时读取到不同内存
    // vec[0]存buffer上
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    // vec[1] 存栈上
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // 如果buffer中剩余可写入数据比栈小就用2块（vec[0]+ vec[1]）否则只用buffer的
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if(n < 0)
    {
        *saveErrno = errno;
    } 
    else if (n <= writable)
    {
        // 如果读出来的数据小于buffer中剩余可写入数据，移动写索引
        writerIndex_ += n;
    }
    else
    {
        // buffer中剩余可写入数据不够了，把栈上数据加进来，底层是调用copy
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

// 从fd中写
ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if(n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}
