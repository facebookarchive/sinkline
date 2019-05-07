/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include "Scheduler.h"

using namespace fb::sinkline;

ThreadScheduler::ThreadScheduler (bool yieldBetweenActions)
  : _state(std::make_shared<State>(yieldBetweenActions))
{
  std::thread([state = _state] {
    detachedThreadMain(state);
  }).detach();
}

void ThreadScheduler::suspend ()
{
  std::lock_guard<std::mutex> guard(_state->_mutex);

  if (_state->_suspensionCount == std::numeric_limits<decltype(_state->_suspensionCount)>::max()) {
    throw std::overflow_error("ThreadScheduler suspension count overflow");
  }

  ++_state->_suspensionCount;
}

void ThreadScheduler::resume ()
{
  std::lock_guard<std::mutex> guard(_state->_mutex);

  if (_state->_suspensionCount == std::numeric_limits<decltype(_state->_suspensionCount)>::min()) {
    throw std::underflow_error("ThreadScheduler suspension count underflow (mismatched suspend/resume)");
  }

  --_state->_suspensionCount;
}

void ThreadScheduler::shutdown ()
{
  {
    std::lock_guard<std::mutex> guard(_state->_mutex);
    _state->_running = false;
  }

  _state->_condition.notify_all();
}

void ThreadScheduler::detachedThreadMain (std::shared_ptr<State> state)
{
  while (true) {
    decltype(state->_queue) actions;
    std::unique_lock<std::mutex> guard(state->_mutex);

    while (state->_queue.empty() || state->_suspensionCount > 0) {
      state->_condition.wait(guard);

      if (!state->_running) {
        return;
      }
    }

    actions.swap(state->_queue);
    guard.unlock();

    for (const auto &action : actions) {
      action();

      if (state->_yieldBetweenActions) {
        std::this_thread::yield();
      }
    }
  }
}
