// Copyright 2025 THALES ALENIA SPACE FRANCE. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef XSMP_THREADSAFEDATA_H_
#define XSMP_THREADSAFEDATA_H_

#include <condition_variable>
#include <mutex>
#include <optional>
#include <shared_mutex>

namespace Xsmp {
template <typename T> class ThreadSafeData {
public:
  // Constructor to initialize the data with the provided arguments
  template <typename... Args>
  explicit ThreadSafeData(Args &&...args)
      : _data(std::forward<Args>(args)...) {}

  // Read lock management class
  class ReadLock {
  public:
    // The shared lock is passed as a parameter to the constructor
    ReadLock(const ThreadSafeData &wrapper,
             std::shared_lock<std::shared_mutex> &&lock)
        : _wrapper(wrapper), _lock(std::move(lock)) {}

    // Access the data
    const T &get() const { return _wrapper._data; }

    // Relock after an unlock
    void lock() {
      _lock = std::shared_lock<std::shared_mutex>(_wrapper._mutex);
    }

    // Unlock the read lock
    void unlock() { _lock.unlock(); }

  private:
    const ThreadSafeData &_wrapper;            // Reference to the wrapper
    std::shared_lock<std::shared_mutex> _lock; // Shared lock
  };

  // Write lock management class
  class WriteLock {
  public:
    // The unique lock is passed as a parameter to the constructor
    WriteLock(ThreadSafeData &wrapper,
              std::unique_lock<std::shared_mutex> &&lock)
        : _wrapper(wrapper), _lock(std::move(lock)) {}

    // Access the data
    T &get() { return _wrapper._data; }

    // Relock after an unlock
    void lock() {
      _lock = std::unique_lock<std::shared_mutex>(_wrapper._mutex);
    }

    // Unlock the write lock
    void unlock() {
      _lock.unlock();
      _wrapper._cv.notify_all(); // Notify all waiting threads
    }

  private:
    ThreadSafeData &_wrapper;                  // Reference to the wrapper
    std::unique_lock<std::shared_mutex> _lock; // Unique lock
  };

  // Blocking read operation (acquires read lock)
  ReadLock read() const {
    return ReadLock(*this, std::shared_lock<std::shared_mutex>(_mutex));
  }

  // Blocking write operation (acquires write lock)
  WriteLock write() {
    return WriteLock(*this, std::unique_lock<std::shared_mutex>(_mutex));
  }

  // Non-blocking read operation (returns nullopt if lock cannot be acquired)
  std::optional<ReadLock> try_read() const {
    if (std::shared_lock lock(_mutex, std::try_to_lock); lock.owns_lock()) {
      return ReadLock(*this, std::move(lock));
    }
    return std::nullopt;
  }

  // Non-blocking write operation (returns nullopt if lock cannot be acquired)
  std::optional<WriteLock> try_write() {
    if (std::unique_lock lock(_mutex, std::try_to_lock); lock.owns_lock()) {
      return WriteLock(*this, std::move(lock));
    }
    return std::nullopt;
  }

private:
  mutable std::shared_mutex _mutex; // Mutex to protect the data
  mutable std::condition_variable_any
      _cv; // Condition variable for synchronization
  T _data; // The actual data
};

} // namespace Xsmp

#endif // XSMP_THREADSAFEDATA_H_