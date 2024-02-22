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

struct Timer
{
  chrono::milliseconds m_duration;
  function<void()> m_callback;
  bool m_repeat;
  int m_priority;
  bool m_running;
  chrono::time_point<chrono::steady_clock> m_endTime;

  template<typename Func, typename... Args>
  Timer(long duration, Func&& func, Args&&... args)
    : m_duration(duration), 
      m_callback(std::bind(std::forward<Func>(func), std::forward<Args>(args)...)), 
      m_repeat(false), 
      m_running(false) {
    m_endTime = chrono::steady_clock::now() + m_duration;
  }

  bool operator<(const Timer &other) const {
    return m_endTime > other.m_endTime;
  }
};
struct TimerCompare {
    bool operator()(const std::shared_ptr<Timer>& t1, const std::shared_ptr<Timer>& t2) const {
        return t1->m_endTime > t2->m_endTime;
    }
};
class TimerManager
{
  priority_queue<shared_ptr<Timer>, vector<shared_ptr<Timer>>, TimerCompare > m_timers;
  // priority_queue<shared_ptr<Timer>> m_timers;
  mutex m_mutex;
  condition_variable m_condition;
  bool m_stop = false;
  thread m_thread;

public:
  TimerManager() {
    m_thread = thread([this] { run(); });
  }

  ~TimerManager() {
    stop();
    m_thread.join();
  }

  void addTimer(shared_ptr<Timer> timer)
  {
    unique_lock<mutex> lock(m_mutex);
    bool notify = m_timers.empty() || timer->m_endTime < m_timers.top()->m_endTime;
    m_timers.push(timer);
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
      while (!m_stop && m_timers.empty()) {
        m_condition.wait(lock);
      }
      if (m_stop ) {
        if (!m_timers.empty()) {
          cout << "TimerManager: " << m_timers.size() << " timers left" << endl;
        }
        return;
      }
      auto timer = m_timers.top();
      auto now = chrono::steady_clock::now();
      if (now < timer->m_endTime) {
        m_condition.wait_until(lock, timer->m_endTime);
        if (now < timer->m_endTime) {
          continue;
        }
      }
      m_timers.pop();
      lock.unlock();
      timer->m_callback();
      if (timer->m_repeat) {
        timer->m_endTime = chrono::steady_clock::now() + timer->m_duration;
        addTimer(timer);
      }
    }
  }
};