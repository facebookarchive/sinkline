/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_SCHEDULER_H
#define FB_SINKLINE_SCHEDULER_H

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

#if __has_include(<dispatch/dispatch.h>)
#include <dispatch/dispatch.h>
#include <sys/time.h>
#endif

#include "PlatformSupport.h"

namespace fb { namespace sinkline {

template<typename F, typename... Args, typename R = std::result_of_t<F &&(Args &&...)>>
void setPromise (std::promise<R> &p, F &&action, Args &&...args)
{
  p.set_value(action(std::forward<Args>(args)...));
}

template<typename F, typename... Args>
void setPromise (std::promise<void> &p, F &&action, Args &&...args)
{
  action(std::forward<Args>(args)...);
  p.set_value();
}

template<typename F, typename... Args, typename R = std::result_of_t<F &&(Args &&...)>>
void runPromisedAction (std::promise<R> &promise, F &&action, Args &&...args)
{
  try {
    setPromise(promise, std::forward<F>(action), std::forward<Args>(args)...);
  } catch (...) {
    promise.set_exception(std::current_exception());
  }
}

/// Implements the behavior of reschedule() for different callable objects.
template<typename Scheduler, typename Callable>
struct RescheduleHelper final : public RescheduleHelper<Scheduler, decltype(&Callable::operator())>
{
  public:
    RescheduleHelper () = delete;
};

template<typename Scheduler, typename Callable, typename Result, typename... Arguments>
struct RescheduleHelper<Scheduler, Result (Callable::*)(Arguments...)>
{
  public:
    RescheduleHelper () = delete;

    static auto reschedule (Scheduler scheduler, Callable fn)
    {
      return [scheduler, fn = std::move(fn)](Arguments ...arguments) {
        scheduler->schedule(fn, arguments...);
      };
    }
};

template<typename Scheduler, typename Result, typename... Arguments>
struct RescheduleHelper<Scheduler, Result (*)(Arguments...)> final
{
  public:
    RescheduleHelper () = delete;

    static auto reschedule (Scheduler scheduler, Result (*fn)(Arguments...))
    {
      return [scheduler, fn](Arguments ...arguments) {
        scheduler->schedule(fn, arguments...);
      };
    }
};

#if __has_feature(blocks)

template<typename Scheduler, typename Result, typename... Arguments>
struct RescheduleHelper<Scheduler, Result (^)(Arguments...)> final
{
  public:
    RescheduleHelper () = delete;

    static auto reschedule (Scheduler scheduler, Result (^block)(Arguments...))
    {
      return ^(Arguments ...arguments) {
        scheduler->schedule(block, arguments...);
      };
    }
};

#endif

/// Given a callable object, creates a new callable object which will invoke the
/// original upon the given scheduler and discard the result.
///
/// If Callable represents an Objective-C block, the return type of this
/// function will be another Objective-C block. Otherwise, it will be a C++
/// lambda.
template<typename Scheduler, typename Callable>
auto reschedule (Scheduler scheduler, Callable &&fn)
{
  return RescheduleHelper<Scheduler, std::remove_reference_t<Callable>>::reschedule(scheduler, std::forward<Callable>(fn));
}

class ThreadScheduler final
{
  public:
    ThreadScheduler (bool yieldBetweenActions = false);

    ThreadScheduler (const ThreadScheduler &) = delete;
    ThreadScheduler &operator= (const ThreadScheduler &) = delete;

    ThreadScheduler (ThreadScheduler &&) = default;
    ThreadScheduler &operator= (ThreadScheduler &&) = default;

    ~ThreadScheduler ()
    {
      if (_state) {
        shutdown();
      }
    }

    template<typename F, typename ...Args>
    std::future<std::result_of_t<F(Args...)>> schedule (F action, Args ...args)
    {
      auto promise = std::make_shared<std::promise<void>>();

      {
        std::lock_guard<std::mutex> guard(_state->_mutex);

        _state->_queue.push_back([promise, action = std::move(action), args...] {
          runPromisedAction(*promise, action, args...);
        });
      }

      _state->_condition.notify_all();
      return promise->get_future();
    }

    void suspend ();
    void resume ();

    void shutdown ();

  private:
    struct State {
      std::mutex _mutex;
      std::condition_variable _condition;
      const bool _yieldBetweenActions;

      // These fields must be synchronized on _mutex.
      std::vector<std::function<void()>> _queue;
      bool _running;
      unsigned _suspensionCount;

      State (bool yieldBetweenActions)
        : _yieldBetweenActions(yieldBetweenActions)
        , _running(true)
        , _suspensionCount(0)
      {}
    };

    std::thread _thread;
    std::shared_ptr<State> _state;

    void startIfNeeded (const std::lock_guard<std::mutex> &guard);
    static void detachedThreadMain (std::shared_ptr<State> state);
};

struct ImmediateScheduler final
{
  public:
    template<typename F, typename ...Args>
    std::future<std::result_of_t<F(Args...)>> schedule (F &&action, Args &&...args)
    {
      std::promise<std::result_of_t<F(Args...)>> promise;

      runPromisedAction(promise, std::forward<F>(action), std::forward<Args>(args)...);

      return promise.get_future();
    }
};

#if DISPATCH_API_VERSION

class GCDScheduler final
{
  public:
    GCDScheduler (dispatch_queue_t targetQueue = NULL, const char *label = "com.facebook.sinkline.GCDScheduler", dispatch_queue_attr_t attr = DISPATCH_QUEUE_SERIAL) noexcept
      : _queue(dispatch_queue_create(label, attr))
    {
      if (targetQueue) {
        dispatch_set_target_queue(_queue, targetQueue);
      }
    }

    GCDScheduler (const GCDScheduler &) = delete;
    GCDScheduler &operator= (const GCDScheduler &) = delete;

    GCDScheduler (GCDScheduler &&) = default;
    GCDScheduler &operator= (GCDScheduler &&) = default;

    static std::shared_ptr<GCDScheduler> mainQueueScheduler()
    {
      return std::make_shared<GCDScheduler>(dispatch_get_main_queue(), "com.facebook.sinkline.GCDScheduler.mainQueueScheduler");
    }

    template<typename F, typename ...Args>
    std::future<std::result_of_t<F(Args...)>> schedule (F action, Args ...args)
    {
      auto promise = std::make_shared<std::promise<std::result_of_t<F(Args...)>>>();

      dispatch_async(_queue, ^{
        runPromisedAction(*promise, action, args...);
      });

      return promise->get_future();
    }

    template<typename Clock, typename F, typename ...Args>
    std::future<std::result_of_t<F(Args...)>> scheduleAfter (std::chrono::time_point<Clock> timePoint, F action, Args ...args)
    {
      auto promise = std::make_shared<std::promise<std::result_of_t<F(Args...)>>>();

      dispatch_block_t actionBlock = ^{
        runPromisedAction(*promise, action, args...);
      };

      if (Clock::is_steady) {
        auto now = Clock::now();
        auto nsDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(timePoint - now);

        dispatch_time_t dsTime = dispatch_time(DISPATCH_TIME_NOW, nsDelta.count());
        dispatch_after(dsTime, _queue, actionBlock);
      } else {
        auto epochDuration = timePoint.time_since_epoch();

        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epochDuration);
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(epochDuration);
        ns -= seconds;

        struct timespec ts;
        ts.tv_sec = seconds.count();
        ts.tv_nsec = ns.count();

        dispatch_time_t dsTime = dispatch_walltime(&ts, 0);
        dispatch_after(dsTime, _queue, actionBlock);
      }

      return promise->get_future();
    }

    void suspend () noexcept
    {
      dispatch_suspend(_queue);
    }

    void resume () noexcept
    {
      dispatch_resume(_queue);
    }

  private:
    dispatch_queue_t _queue;
};

#endif

} } // namespace fb::sinkline

#endif
