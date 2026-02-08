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
  bool operator()(const unique_ptr<Task>& t1, const unique_ptr<Task>& t2) const {
    return t1->m_priority > t2->m_priority;
  }
};

class TaskManager
{
  priority_queue<unique_ptr<Task>, vector<unique_ptr<Task>>, TaskCompare> m_tasks;
  mutex m_mutex;
  condition_variable m_condition;
  bool m_stop = false;
  vector<thread> m_thread;
public:
  TaskManager() {
    int numberOfThreads = static_cast<int>(thread::hardware_concurrency());
    if (numberOfThreads <= 0) {
      numberOfThreads = 1;
    }
    for (int i = 0; i < numberOfThreads; i++) {
      m_thread.push_back(thread([this] { 
        while (true) {
          unique_lock<mutex> lock(m_mutex);
          m_condition.wait(lock, [this] { return m_stop || !m_tasks.empty(); });
          if (m_stop && m_tasks.empty()) {
            return;
          }
          auto task = std::move(const_cast<unique_ptr<Task>&>(m_tasks.top()));
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
  
  void addTask(unique_ptr<Task> task)
  {
    if (!task) {
      return;
    }
    unique_lock<mutex> lock(m_mutex);
    if (m_stop) {
      return;
    }
    m_tasks.push(std::move(task));
    lock.unlock();
    m_condition.notify_one();
  }

  void stop() {
    unique_lock<mutex> lock(m_mutex);
    m_stop = true;
    lock.unlock();
    m_condition.notify_all();
    for (auto &t : m_thread) {
      if (t.joinable()) {
        t.join();
      }
    }
  }
};
