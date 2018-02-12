//
// Created by hjzh on 18-2-11.
//

#ifndef VIDEODETECTDEMO_NAMEDMUTEXSCOPEDLOCK_H
#define VIDEODETECTDEMO_NAMEDMUTEXSCOPEDLOCK_H

#include "Poco/NamedMutex.h"

class NamedMutexScopedLock {
public:
  explicit NamedMutexScopedLock(Poco::NamedMutex& mutex): _mutex(mutex) {

  }

  bool tryLock() {
    return _mutex.tryLock();
  }

  void lock() {
    _mutex.lock();
  }

  virtual ~NamedMutexScopedLock() {
    _mutex.unlock();
  }

private:
  Poco::NamedMutex& _mutex;

  NamedMutexScopedLock() = delete;
  NamedMutexScopedLock(const NamedMutexScopedLock&) = delete;
  NamedMutexScopedLock& operator= (const NamedMutexScopedLock&);
};
#endif //VIDEODETECTDEMO_NAMEDMUTEXSCOPEDLOCK_H
