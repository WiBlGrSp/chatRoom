#include "../include/threadPool.h"

// 构造函数定义
ThreadPool::ThreadPool(int thread_pool_size) : stop_(false)
{
    startThreadPool(thread_pool_size); // 启动线程池
}

// 析构函数的定义
ThreadPool::~ThreadPool()
{
    {
        unique_lock<mutex> lock(task_mutex_); // 获取锁资源，用于保护stop变量
        stop_ = true;
    }

    // 将所有的等待线程唤醒
    task_cv_.notify_all();

    // 将所有的工作线程回收
    for (auto &worker : workers_)
    {
        worker.join();
    }
}

// 将任务加到线程池中
void ThreadPool::addTask(function<void()> task)
{
    {
        unique_lock<mutex> lock(task_mutex_); // 获取锁资源，保护条件变量
        tasks_.push(task);                    // 将任务添加到任务队列
    }

    // 唤醒一个等待的线程开始工作
    task_cv_.notify_one(); // 唤醒一个线程去工作
}

// 启动线程池函数的定义
void ThreadPool::startThreadPool(size_t num_threads)
{
    // 循环创建num_threads个线程
    for (int i = 0; i < num_threads; i++)
    {
        // 创建一个线程，并且将线程放入到线程容器中
        workers_.emplace_back([this] // 使用lambda表达式定义线程体
            {
            while(true)
            {
                function<void()> task;    //创建一个任务
                {
                    unique_lock<mutex> lock(task_mutex_);     //加锁，保护任务队列
                    task_cv_.wait(lock, [this]{       //等待条件变量通知
                        return stop_ || !tasks_.empty(); //当线程池停止工作或者任务队列不为空时
                    });

                    if(stop_ && tasks_.empty())
                    {
                        return;              //当线程池停止或者任务队列为空，退出线程
                    }

                    task = std::move(tasks_.front());     //从任务队列中取出一个任务
                    tasks_.pop();                   //将任务从任务队列中移除 
                }
                task();           //执行任务
            } });
    }
}