#pragma once
#include <ctime>
#include <iostream>
#include <functional>
#include <memory>

#include "Timestamp.h"

class EventLoop;

class Channel
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);

    void setReaCallback(ReadEventCallback cb) { readCallback = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback = std::move(cb); }

    void tie(const std::shared_ptr<void> &);

    inline int fd() const {return fd_;}
    inline int events() const {return events_;}
    inline int revents() const {return revents_;}
    inline int index() const {return index_;}



    inline void set_index(int index) { index_ = index; }

    // 设置channel的感兴趣事件，调用上层channel的update，update调用底层的epoll_ctl
    void enableReading() { events_ |= kReadEvent, update(); }
    void disableReading() { events_ &= ~kReadEvent, update(); }
    void enableWriting() { events_ |= kReadEvent, update(); }
    void disableWriting() { events_ &= ~kWriteEvent, update(); }

    // 返回fd当前事件的状态
    inline bool isNoneEvent() const { return events_ == kNoneEvent; }
    inline bool isReadEvent() const { return events_ & kReadEvent; }
    inline bool isWriteEvent() const { return events_ & kWriteEvent; }

    // 返回所属的loop   
    EventLoop* ownerLoop() { return loop_; }
    void remove();

private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    
    EventLoop* loop_; // 所属的LoopEvent
    const int fd_; // 对应的fd
    int events_; // fd上注册的感兴趣的事件
    int revents_; // poller返回的具体发生的事件
    int index_; //

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback readCallback;
    EventCallback writeCallback;
    EventCallback closeCallback;
    EventCallback errorCallback;
};