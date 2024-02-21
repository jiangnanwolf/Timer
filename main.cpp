#include "timer.h"

#include <chrono>
#include <iostream>

int main() {

  TimerManager timerManager;
  timerManager.addTimer(make_shared<Timer>(chrono::milliseconds(5000), []() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&now_c) << std::endl;

    cout << "Timer 5" << endl;
  }, false));
  timerManager.addTimer(make_shared<Timer>(chrono::milliseconds(2000), []() {
        auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&now_c) << std::endl;

    cout << "Timer 2" << endl;
  }, false));

    timerManager.addTimer(make_shared<Timer>(chrono::milliseconds(1000), []() {
          auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&now_c);

    cout << "Timer 1" << endl;
  }, true));
  timerManager.addTimer(make_shared<Timer>(chrono::milliseconds(3000), []() {
        auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&now_c) << std::endl;

    cout << "Timer 3" << endl;
  }, false));

  this_thread::sleep_for(chrono::seconds(10));

  return 0;
}