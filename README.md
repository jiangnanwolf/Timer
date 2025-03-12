# Timer

## Description
This project implements a timer and task management system using C++ with multithreading support. It allows scheduling of tasks to be executed after a specified duration, with support for repeating tasks and prioritization.

## Features
- Schedule tasks with a specified delay.
- Support for repeating tasks.
- Task prioritization.
- Multithreaded task execution.

## Build Instructions
To build the project, you need CMake and a C++ compiler that supports C++14. Follow these steps:

1. Clone the repository:
   ```sh
   git clone <repository_url>
   cd Timer
   ```

2. Create a build directory and navigate into it:
   ```sh
   mkdir build
   cd build
   ```

3. Run CMake to generate the build files:
   ```sh
   cmake ..
   ```

4. Build the project:
   ```sh
   cmake --build .
   ```

## Usage
After building the project, you can run the executable to see the timer and task manager in action.

```sh
./timer
```

The `main.cpp` file contains examples of how to create and add timers to the `TimerManager`.

## Example
Here is a simple example of scheduling a task to print a message after 8 seconds:

```cpp
#include "timer.h"
#include "task.h"

void printMessage(const std::string &message)
{
  std::cout << message << std::endl;
}

int main()
{
  TimerManager timerManager;

  auto timer = std::make_shared<Timer>(8000, printMessage, "Hello, world!");
  timerManager.addTimer(timer);

  this_thread::sleep_for(chrono::seconds(10));

  return 0;
}
```

## License
This project is licensed under the MIT License.
