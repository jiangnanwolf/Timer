#pragma once

#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

using namespace std;

struct Task
{
  int m_priority;
  function<void()> m_callback;
  template<typename Func, typename... Args>
  Task(Func&& func, Args&&... args)
    : m_priority(0),
      m_callback(std::bind(std::forward<Func>(func), std::forward<Args>(args)...)) {
  }
};

struct TaskCompare {
  bool operator()(const shared_ptr<Task>& t1, const shared_ptr<Task>& t2) const {
    return t1->m_priority > t2->m_priority;
  }
};

class TaskManager
{
  priority_queue<shared_ptr<Task>> m_tasks;
  mutex m_mutex;
  condition_variable m_condition;
  bool m_stop = false;
  vector<thread> m_thread;
public:
  TaskManager() {
    int numberOfThreads = thread::hardware_concurrency();
    for (int i = 0; i < numberOfThreads; i++) {
      m_thread.push_back(thread([this] { 
        while(!m_stop) {
          unique_lock<mutex> lock(m_mutex);
          m_condition.wait(lock, [this] { return m_stop || !m_tasks.empty(); });
          if (m_stop) {
            return;
          }
          auto task = m_tasks.top();
          m_tasks.pop();
          lock.unlock();
          task->m_callback();
        }
      }));
    }
  }

  ~TaskManager() {
    stop();
  }
  
  void addTask(shared_ptr<Task> task)
  {
    unique_lock<mutex> lock(m_mutex);
    m_tasks.push(task);
    m_condition.notify_one();
  }

  void stop() {
    m_stop = true;
    m_condition.notify_all();
    for (auto &t : m_thread) {
      t.join();
    }
  }
};