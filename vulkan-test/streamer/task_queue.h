//
// Created by admin on 2025/11/4.
//

#ifndef ZETAENGINE_TASK_QUEUE_H
#define ZETAENGINE_TASK_QUEUE_H

#include "helpers/helpers.h"

#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
namespace task
{
class TaskQueue {
public:
    using Task = std::function<void()>;

    TaskQueue();

    ~TaskQueue();

    // 添加任务到队列
    template<typename F, typename... Args>
    void enqueue(F&& f, Args&&... args);

    // 执行队列中的任务（阻塞调用）
    void execute();
    // 执行单个任务（如果有任务的话）
    bool executeOne();

    // 执行指定数量的任务
    size_t executeBatch(size_t maxTasks);

    // 执行所有当前存在的任务（非阻塞）
    size_t executeAll();

    // 等待并执行任务，带超时机制
    bool executeWithTimeout(std::chrono::milliseconds timeout);
    // 停止任务队列
    void stop();

    // 获取队列大小
    size_t size() const;

    // 检查队列是否为空
    bool empty() const;

    void blockUntilNotEmpty();
private:
    std::queue<Task> tasks_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
};
enum EThreadType
{
    GameThread,
    Unknown
};

class TaskScheduler
{
public:
    //using Alias = std::variant<std::string, EThreadType>;
    using mutex = std::recursive_mutex;
    using lock_guard = std::lock_guard<std::recursive_mutex>;
    TaskScheduler();
    ~TaskScheduler();
    void setThreadAlias(std::thread::id id, std::string_view alias);
    bool removeThreadAlias(std::thread::id id);
    // 为当前线程创建任务队列
    void registerCurrentThread(std::string_view alias = {});

    // 为指定线程创建任务队列
    void registerThread(std::thread::id threadId, std::string_view alias = {});

    // 向指定线程的任务队列添加任务
    bool enqueueToThread(std::thread::id threadId, TaskQueue::Task task);
    // 向指定线程的任务队列添加任务
    bool enqueueToThread(EThreadType ThreadType, TaskQueue::Task task);
    bool enqueueToThread(std::string_view name, TaskQueue::Task task);

    // 获取当前线程的任务队列
    TaskQueue* getCurrentThreadQueue();
    void blockCurrentUntilQueueNotEmpty();

    // 执行当前线程的一批任务
    size_t executeCurrentThreadBatch(size_t maxTasks = 1);
    bool executeCurrentThreadWithTimeout(std::chrono::milliseconds timeout);
    // 执行当前线程的所有任务
    size_t executeCurrentThreadAll();

    // 执行指定线程的单个任务
    bool executeThreadOne(std::thread::id threadId) const;

    // 获取所有注册的线程ID
    std::vector<std::thread::id> getRegisteredThreads() const;

    // 获取任务队列数量
    size_t getQueueCount() const;
    // 获取任务队列数量
    size_t getQueueTaskCount(std::string_view name) const;
    size_t getQueueTaskCount(std::thread::id id)const;
    size_t getQueueTaskCount(EThreadType ThreadType)const;
    // 停止所有任务队列
    void stopAll() const;

    // 移除指定线程的任务队列
    bool unregisterThread(std::thread::id threadId);

    // 移除当前线程的任务队列
    bool unregisterCurrentThread();


private:
    bool enqueueToThread_inl(std::thread::id threadId, TaskQueue::Task task);
    // 获取指定线程的任务队列，危险！！
    TaskQueue* getThreadQueue(std::thread::id threadId) const;
    mutable mutex mutex_;
    std::unordered_map<std::thread::id, std::unique_ptr<TaskQueue>> threadQueues_;
    std::unordered_map<std::string, std::thread::id> aliasThread_;
    std::unordered_map<std::thread::id, std::string> threadAlias_;
};

extern TaskScheduler g_TaskScheduler;
inline bool AsyncTask(std::string_view alias, TaskQueue::Task task)
{
    helpers::log_info("start enqueue task");
    bool b = g_TaskScheduler.enqueueToThread(alias, std::move(task));
    helpers::log_info("end enqueue task");
    return b;
}
inline bool AsyncTask(EThreadType type, TaskQueue::Task task)
{
    return g_TaskScheduler.enqueueToThread(type, std::move(task));
}

inline bool AsyncTask(std::thread::id thread_id, TaskQueue::Task task)
{
    return g_TaskScheduler.enqueueToThread(thread_id, std::move(task));
}
}



#endif // ZETAENGINE_TASK_QUEUE_H
