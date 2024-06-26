// Copyright 2023 YOUR ORGANIZATION. All rights reserved.
//
// YOUR NOTICE

// -----------------------------------------------------------------------------
// File Name    : CounterManager.cpp
// Generated by : XsmpSdkGenerator-${qualifiedVersion}
// -----------------------------------------------------------------------------
/// @file Example/CounterManager.cpp

#include <Example/CounterManager.h>
#include <numeric>
namespace Example {
void CounterManager::DoPublish(::Smp::IPublication *) {}

void CounterManager::DoConfigure(::Smp::Services::ILogger *,
                                 ::Smp::Services::ILinkRegistry *) {}

void CounterManager::DoConnect(::Smp::ISimulator *) {}

void CounterManager::DoDisconnect() {}

::Smp::Int32 CounterManager::get_sum() {
  return std::accumulate(counters.begin(), counters.end(), 0,
                         [](::Smp::Int32 count, Counter *counter) {
                           return count + counter->count;
                         });
}
::Smp::Float64 CounterManager::get_average() {
  if (!counters.empty())
    return static_cast<::Smp::Float64>(get_sum()) /
           static_cast<::Smp::Float64>(counters.size());
  return 0;
}
void CounterManager::set_count(::Smp::Int32 value) {
  for (auto *counter : counters)
    counter->count = value;
}
} // namespace Example
