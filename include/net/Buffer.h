#pragma once

#include "Logging.h"
#include <algorithm>
#include <stddef.h>
#include <vector>
#include <string>

// 封装buffer， 底层是vector
class Buffer
{
public:   
    static const size_t kCheapPrepend = 8;
    // kInitialSize 默认初始化长度
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {}

    // 可读取数据长度
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    // 剩余可写数据长度
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    // 开始读的位置
    const char* peek() const {return begin() + readerIndex_; }

    // 取出len个数据
    void retrieve(size_t len)
    {
        if(len < readableBytes())
        {
            readerIndex_ += len;
        }
        else 
        {
            retrieveAll();
        }
    }

    // 复原读写索引
    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    // 按string取出所有
    std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }
    // 按string取出len个数据
    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    // 确保可以写len个数据, 不可以写就扩容
    void ensureWriteableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            makeSpace(len);
        }
    }

    // 将len长度datacopy到当前的数据后面
    void append(const char* data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data+len, beginWrite());
        writerIndex_ += len;
    }

    // void append(const char* data) {
    //     size_t len = sizeof(len);
    //     append(data, len);
    // }

    void append(const std::string &str)
    {
        append(str.data(), str.size());
    }

    // 开始写位置指针
    char* beginWrite() { return begin() + writerIndex_; }
    // const char*
    const char* beginWrite() const { return begin() + writerIndex_; }

    bool empty() const {
        return readableBytes() == 0 ;
    }

    const char* findCRLF() const {
        if(empty()) return nullptr;
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+1);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    void retrieveUntil(const char* end) {
        if(end < peek() || end > beginWrite()) {
            LOG_FATAL << "retrieveUntil error";
        }
        retrieve(end - peek());
    }

    ssize_t readFd(int fd, int *saveErrno);
    ssize_t writeFd(int fd, int *saveErrno);

private:

    // begin指针
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }
    
    // 扩容
    void makeSpace(size_t len)
    {   
        /*
            改写成这样就好理解了
            writableBytes + (prependableBytes - kCheapPrepend) <> len
            比较buffer中空闲的位置 （可写 + 读索引到kCheapPrepend(空白)）和 len的大小
        */  
        // 空闲位置不够
        if(writableBytes() + prependableBytes() < len + kCheapPrepend)
        {   
            // 扩容
            buffer_.resize(writerIndex_ + len);
        }
        else
        {   
            // 空闲位置足够，把readidx ~ writeidx 部分拷贝到前面
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];
};