#ifndef SCOPED_TIMER_H
#define SCOPED_TIMER_H

#include <chrono>
#include <memory>
#include <string>

// base class for ScopedTimers; override destructor and use GetElapsedSeconds()
class ScopedTimer {
public:
  ScopedTimer();
  virtual ~ScopedTimer() {}
  double GetElapsedSeconds();

private:
  std::chrono::steady_clock::time_point start_;
};

// a ScopedTimer which prints to cout on destruction
class PrintingScopedTimer : public ScopedTimer {
public:
  PrintingScopedTimer(std::string msg);
  virtual ~PrintingScopedTimer();

private:
  std::string msg_;
};

/*
AccumulatingScopedTimer adds its time to a total on destruction.

Example usage:

  auto foo_timer_accumulator = AccumulatingScopedTimer::MakeAccumulator();
  auto bar_timer_accumulator = AccumulatingScopedTimer::MakeAccumulator();

  for(...) {
    {
      AccumulatingScopedTimer foo_timer(foo_timer_accumulator)
      ...do foo...
    }

    {
      AccumulatingScopedTimer bar_timer(bar_timer_accumulator)
      ...do bar...
    }
  }

  cout << "total time spent on foo: " << *foo_timer_accumulator
    << " and bar: " << *bar_timer_accumulator << endl;
*/
class AccumulatingScopedTimer : public ScopedTimer {
public:
  static std::shared_ptr<double> MakeAccumulator();

  AccumulatingScopedTimer(std::shared_ptr<double> accumulator);
  virtual ~AccumulatingScopedTimer();

private:
  std::shared_ptr<double> accumulator_;
};

#endif
