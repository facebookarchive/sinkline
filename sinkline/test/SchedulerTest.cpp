/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include "TestCommon.h"

#include <sinkline/Scheduler.h>

#include <chrono>
#include <thread>

using namespace fb::sinkline;

TEST(SchedulerTest, ThreadScheduler)
{
  ThreadScheduler s;

  auto promise = std::make_shared<std::promise<void>>();

  s.schedule([=] {
    promise->set_value();
  });

  auto result = promise->get_future().wait_for(std::chrono::seconds(1));
  EXPECT_EQ(result, std::future_status::ready);
}

#if DISPATCH_API_VERSION

TEST(SchedulerTest, GlobalGCDScheduler)
{
  GCDScheduler s;

  auto promise = std::make_shared<std::promise<void>>();

  s.scheduleAfter(std::chrono::steady_clock::now() + std::chrono::microseconds(1), [=] {
    promise->set_value();
  });

  auto result = promise->get_future().wait_for(std::chrono::seconds(1));
  EXPECT_EQ(result, std::future_status::ready);
}

#endif
