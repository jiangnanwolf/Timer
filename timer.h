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

#include "task.h"

using namespace std;

struct Timer
{
  chrono::milliseconds m_duration;
  function<void()> m_callback;
  bool m_repeat;
  int m_priority;
  bool m_running;
  bool m_longRun;
  chrono::time_point<chrono::steady_clock> m_endTime;

  template<typename Func, typename... Args>
  Timer(long duration, Func&& func, Args&&... args)
    : m_duration(duration), 
      m_callback(std::bind(std::forward<Func>(func), std::forward<Args>(args)...)), 
      m_repeat(false), 
      m_running(false),
      m_longRun(false) {
    m_endTime = chrono::steady_clock::now() + m_duration;
  }

  bool operator<(const Timer &other) const {
    return m_endTime > other.m_endTime;
  }
};
struct TimerCompare {
    bool operator()(const std::unique_ptr<Timer>& t1, const std::unique_ptr<Timer>& t2) const {
        return t1->m_endTime > t2->m_endTime;
    }
};
class TimerManager
{
  priority_queue<unique_ptr<Timer>, vector<unique_ptr<Timer>>, TimerCompare > m_timers;
  mutex m_mutex;
  condition_variable m_condition;
  bool m_stop = false;
  thread m_thread;
  TaskManager& taskManager;
public:
  TimerManager(TaskManager& taskMgr) : taskManager(taskMgr) {
    m_thread = thread([this] { run(); });
  }

  ~TimerManager() {
    stop();
    if (m_thread.joinable()) {
      m_thread.join();
    }
  }

  void addTimer(unique_ptr<Timer> timer)
  {
    if (!timer) {
      return;
    }
    unique_lock<mutex> lock(m_mutex);
    if (m_stop) {
      return;
    }
    bool notify = m_timers.empty() || timer->m_endTime < m_timers.top()->m_endTime;
    m_timers.push(std::move(timer));
    lock.unlock();
    if (notify) {
      m_condition.notify_one();
    }
  }

  void stop() {
    unique_lock<mutex> lock(m_mutex);
    m_stop = true;
    lock.unlock();
    m_condition.notify_one();
  }

  void run()
  {
    while (true)
    {
      unique_lock<mutex> lock(m_mutex);
      m_condition.wait(lock, [this] { return m_stop || !m_timers.empty(); });
      if (m_stop && m_timers.empty()) {
        if (!m_timers.empty()) {
          cout << "TimerManager: " << m_timers.size() << " timers left" << endl;
        }
        return;
      }

      while (!m_timers.empty()) {
        auto now = chrono::steady_clock::now();
        auto nextTime = m_timers.top()->m_endTime;
        if (now >= nextTime) {
          break;
        }
        m_condition.wait_until(lock, nextTime);
        if (m_stop && m_timers.empty()) {
          return;
        }
      }

      if (m_timers.empty()) {
        continue;
      }

      auto now = chrono::steady_clock::now();
      if (now < m_timers.top()->m_endTime) {
        continue;
      }

      auto timer = std::move(const_cast<unique_ptr<Timer>&>(m_timers.top()));
      m_timers.pop();
      lock.unlock();

      if (timer->m_longRun) {
        taskManager.addTask(make_unique<Task>(timer->m_callback));
      } else {
        timer->m_callback();
      }

      if (timer->m_repeat) {
        timer->m_endTime = chrono::steady_clock::now() + timer->m_duration;
        addTimer(std::move(timer));
      }
    }
  }
};
