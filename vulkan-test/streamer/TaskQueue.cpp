//
// Created by admin on 2025/11/4.
//

#include "TaskQueue.h"
#include <chrono>
namespace task
{
    TaskScheduler g_TaskScheduler;
TaskQueue::TaskQueue() : stop_(false) {}
TaskQueue::~TaskQueue() { stop(); }
template <typename F, typename... Args>
void TaskQueue::enqueue(F&& f, Args&&... args)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    }
    condition_.notify_one();
}
void TaskQueue::execute()
{
    while (true)
    {
        Task task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty())
            {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}
bool TaskQueue::executeOne()
{
    Task task;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tasks_.empty())
        {
            return false;
        }
        task = std::move(tasks_.front());
        tasks_.pop();
    }

    task();
    return true;
}
size_t TaskQueue::executeBatch(size_t maxTasks)
{
    std::vector<Task> tasks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks.reserve(std::min(maxTasks, tasks_.size()));
        while (tasks.size() < maxTasks && !tasks_.empty())
        {
            tasks.push_back(std::move(tasks_.front()));
            tasks_.pop();
        }
    }
    for (auto& task : tasks)
        task();

    return tasks.size();
}
size_t TaskQueue::executeAll() { return executeBatch(std::numeric_limits<size_t>::max()); }
bool TaskQueue::executeWithTimeout(std::chrono::milliseconds timeout)
{
    const auto tp = std::chrono::system_clock::now();
    while (std::chrono::system_clock::now() - tp < timeout && !empty() && !stop_)
    {
        executeOne();
    }
    return empty();
}
void TaskQueue::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    condition_.notify_all();
}
size_t TaskQueue::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_.size();
}
bool TaskQueue::empty() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_.empty();
}
void TaskQueue::blockUntilNotEmpty()
{
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
}
TaskScheduler::TaskScheduler() = default;
TaskScheduler::~TaskScheduler() { stopAll(); }
void TaskScheduler::registerCurrentThread(std::string_view alias)
{
    const std::thread::id threadId = std::this_thread::get_id();
    registerThread(threadId, alias);
}
void TaskScheduler::registerThread(std::thread::id threadId, std::string_view alias)
{
    lock_guard lock(mutex_);
    if (threadQueues_.find(threadId) == threadQueues_.end())
    {
        threadQueues_[threadId] = std::make_unique<TaskQueue>();
    }

    if (!alias.empty())
        setThreadAlias(threadId, alias);
}
bool TaskScheduler::enqueueToThread(std::thread::id threadId, TaskQueue::Task task)
{
    lock_guard lock(mutex_);
    enqueueToThread_inl(threadId, std::move(task));
    return false;
}
bool TaskScheduler::enqueueToThread(EThreadType ThreadType, TaskQueue::Task task)
{
    lock_guard lock(mutex_);
    const auto result = aliasThread_.find(std::string(magic_enum::enum_name(ThreadType)));
    if (result == aliasThread_.end())
    {
        return false;
    }
    const auto threadId = result->second;

    enqueueToThread_inl(threadId, std::move(task));
    return false;
}
bool TaskScheduler::enqueueToThread(std::string_view name, TaskQueue::Task task)
{
    if (name.empty())
        return false;
    lock_guard lock(mutex_);
    const auto result = aliasThread_.find(std::string(name));
    if (result == aliasThread_.end())
    {
        return false;
    }

    const auto threadId = result->second;
    enqueueToThread_inl(threadId, std::move(task));
    return false;
}
TaskQueue* TaskScheduler::getCurrentThreadQueue()
{
    lock_guard lock(mutex_);
    auto it = threadQueues_.find(std::this_thread::get_id());
    return it != threadQueues_.end() ? it->second.get() : nullptr;
}
void TaskScheduler::blockCurrentUntilQueueNotEmpty()
{
    if (const auto q = getCurrentThreadQueue())
    {
        q->blockUntilNotEmpty();
    }
}
size_t TaskScheduler::executeCurrentThreadBatch(size_t maxTasks)
{
    auto queue = getCurrentThreadQueue();
    return queue ? queue->executeBatch(maxTasks) : 0;
}
bool TaskScheduler::executeCurrentThreadWithTimeout(std::chrono::milliseconds timeout)
{
    const auto queue = getCurrentThreadQueue();
    return queue ? queue->executeWithTimeout(timeout) : 0;
}
size_t TaskScheduler::executeCurrentThreadAll()
{
    auto queue = getCurrentThreadQueue();
    return queue ? queue->executeAll() : 0;
}
bool TaskScheduler::executeThreadOne(std::thread::id threadId)
{
    auto queue = getThreadQueue(threadId);
    return queue ? queue->executeOne() : false;
}
std::vector<std::thread::id> TaskScheduler::getRegisteredThreads() const
{
    lock_guard lock(mutex_);
    std::vector<std::thread::id> threads;
    for (const auto& pair : threadQueues_)
    {
        threads.push_back(pair.first);
    }
    return threads;
}
size_t TaskScheduler::getQueueCount() const
{
    lock_guard lock(mutex_);
    return threadQueues_.size();
}
void TaskScheduler::stopAll()
{
    lock_guard lock(mutex_);
    for (auto& pair : threadQueues_)
    {
        pair.second->stop();
    }
}
bool TaskScheduler::unregisterThread(std::thread::id threadId)
{
    lock_guard lock(mutex_);
    removeThreadAlias(threadId);
    return threadQueues_.erase(threadId) > 0;
}
bool TaskScheduler::unregisterCurrentThread()
{

    return unregisterThread(std::this_thread::get_id());
}
bool TaskScheduler::enqueueToThread_inl(std::thread::id threadId, TaskQueue::Task task)
{
    auto it = threadQueues_.find(threadId);
    if (it != threadQueues_.end())
    {
        it->second->enqueue(std::move(task));
        return true;
    }
    return false;
}
TaskQueue* TaskScheduler::getThreadQueue(std::thread::id threadId)
{
    lock_guard lock(mutex_);
    auto it = threadQueues_.find(threadId);
    return it != threadQueues_.end() ? it->second.get() : nullptr;
}
void TaskScheduler::setThreadAlias(std::thread::id id, std::string_view alias)
{
    lock_guard lock(mutex_);
    if (threadAlias_.contains(id) || aliasThread_.contains(std::string{alias}))
    {
        // throw std::runtime_error("duplicate naming!!");
    }
    else
    {
        threadAlias_.try_emplace(id, alias);
        aliasThread_.try_emplace({alias.data(), alias.length()}, id);
    }
}
bool TaskScheduler::removeThreadAlias(std::thread::id id)
{
    if (const auto it = threadAlias_.find(id); it != threadAlias_.end())
    {
        return aliasThread_.erase(it->second) && threadAlias_.erase(it) != threadAlias_.end();
    }
    return false;
}
}
