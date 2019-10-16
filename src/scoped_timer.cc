#include "scoped_timer.h"

#include <iostream>

ScopedTimer::ScopedTimer() {
  start_ = std::chrono::steady_clock::now();
}

double ScopedTimer::GetElapsedSeconds() {
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::duration<double>>(now - start_).count();
}

PrintingScopedTimer::PrintingScopedTimer(std::string msg)
  : msg_(std::move(msg)) {}

PrintingScopedTimer::~PrintingScopedTimer() {
  double elapsed = GetElapsedSeconds();
  std::cout << msg_ << ' ' << elapsed << std::endl;
}

/*static*/ std::shared_ptr<double> AccumulatingScopedTimer::MakeAccumulator() {
  return std::make_shared<double>(0);
}

AccumulatingScopedTimer::AccumulatingScopedTimer(
  std::shared_ptr<double> accumulator)
  : accumulator_(std::move(accumulator)) {}

AccumulatingScopedTimer::~AccumulatingScopedTimer() {
  double elapsed = GetElapsedSeconds();
  (*accumulator_) += elapsed;
}
