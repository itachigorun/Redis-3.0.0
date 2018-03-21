/* A simple event-driven programming library. Originally I wrote this code
 * for the Jim's event-loop (Jim is a Tcl interpreter) but later translated
 * it in form of a library for easy reuse.
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __AE_H__
#define __AE_H__

// 事件执行状态
// 成功
#define AE_OK 0
// 出错
#define AE_ERR -1

// 文件事件状态
// 未设置
#define AE_NONE 0
// 可读
#define AE_READABLE 1
// 可写
#define AE_WRITABLE 2

// 时间处理器到执行flags
// 文件事件
#define AE_FILE_EVENTS 1
// 时间事件
#define AE_TIME_EVENTS 2
// 所有事件
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
// 不阻塞，也不进行等待
#define AE_DONT_WAIT 4

// 决定时间事件是否要持续执行到flag
#define AE_NOMORE -1

/* Macros */
#define AE_NOTUSED(V) ((void) V)

// 事件处理器状态
struct aeEventLoop;

// 事件接口
/* Types and data structures */
typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData);
typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

// 文件事件结构
/* File event structure */
typedef struct aeFileEvent {
    // 监听事件类型掩码，值可以时 AE_READABLE或AE_WRITABLE,
    // 或者 AE_READABLE | AE_WRITABLE
    int mask; /* one of AE_(READABLE|WRITABLE) */

    // 读事件处理器
    aeFileProc *rfileProc;

    // 写事件处理器
    aeFileProc *wfileProc;

    // 多路复用库到私有数据
    void *clientData;
} aeFileEvent;

// 时间事件结构
/* Time event structure */
typedef struct aeTimeEvent {

    // 时间事件到唯一标识符
    long long id; /* time event identifier. */

    // 事件到到达时间
    long when_sec; /* seconds */
    long when_ms; /* milliseconds */

    // 事件处理函数
    aeTimeProc *timeProc;

    // 事件释放函数
    aeEventFinalizerProc *finalizerProc;

    // 多路复用库到私有数据
    void *clientData;

    // 指向下个时间事件结构，形成链表
    struct aeTimeEvent *next;
} aeTimeEvent;

// 已就绪事件
/* A fired event */
typedef struct aeFiredEvent {

    // 已就绪文件描述符
    int fd;

    // 事件类型掩码，值可以时AE_READABLE 或 AE_WRITABLE 或者是两者到或
    int mask;
} aeFiredEvent;


// 事件处理器到状态
/* State of an event based program */
typedef struct aeEventLoop {

    // 目前已注册到最大描述符
    int maxfd;   /* highest file descriptor currently registered */

    // 目前已追踪到最大描述符
    int setsize; /* max number of file descriptors tracked */

    // 用于生成时间事件 id
    long long timeEventNextId;

    // 最后一次执行时间事件的时间
    time_t lastTime;     /* Used to detect system clock skew */

    // 已注册到文件事件
    aeFileEvent *events; /* Registered events */

    // 已就绪到文件事件
    aeFiredEvent *fired; /* Fired events */

    // 时间事件
    aeTimeEvent *timeEventHead;

    // 事件处理器到开关
    int stop;

    // 多路复用库到私有数据
    void *apidata; /* This is used for polling API specific data */

    // 在处理事件前要执行到函数
    aeBeforeSleepProc *beforesleep;
} aeEventLoop;

/* Prototypes */
aeEventLoop *aeCreateEventLoop(int setsize);
void aeDeleteEventLoop(aeEventLoop *eventLoop);
void aeStop(aeEventLoop *eventLoop);
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
        aeFileProc *proc, void *clientData);
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask);
int aeGetFileEvents(aeEventLoop *eventLoop, int fd);
long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds,
        aeTimeProc *proc, void *clientData,
        aeEventFinalizerProc *finalizerProc);
int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id);
int aeProcessEvents(aeEventLoop *eventLoop, int flags);
int aeWait(int fd, int mask, long long milliseconds);
void aeMain(aeEventLoop *eventLoop);
char *aeGetApiName(void);
void aeSetBeforeSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *beforesleep);
int aeGetSetSize(aeEventLoop *eventLoop);
int aeResizeSetSize(aeEventLoop *eventLoop, int setsize);

#endif
