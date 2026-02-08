#include "timer.h"
#include "task.h"

void printMessage(const std::string& message)
{
  std::cout << message << std::endl;
}

int main()
{
  TaskManager taskManager;
  TimerManager timerManager(taskManager);

  auto timer = std::make_unique<Timer>(8000, printMessage, "Hello, world!");
  auto timer2 = make_unique<Timer>(5000, []() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&now_c) << std::endl;

    cout << "Timer 5" << endl; });
  timerManager.addTimer(std::move(timer));
  timerManager.addTimer(std::move(timer2));
  timerManager.addTimer(make_unique<Timer>(2000, []() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&now_c) << std::endl;

    cout << "Timer 2" << endl; }));
  auto timer3 = make_unique<Timer>(1000, []() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&now_c);

    cout << "Timer 1" << endl; });
  timer3->m_repeat = true;
  timerManager.addTimer(std::move(timer3));
  timerManager.addTimer(make_unique<Timer>(3000, []() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&now_c) << std::endl;

    cout << "Timer 3" << endl; }));

  this_thread::sleep_for(chrono::seconds(10));

  return 0;
}
