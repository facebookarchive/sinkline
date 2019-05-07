/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include <sinkline/Sinkline.h>
#include <sinkline/Scheduler.h>

#include <cstdio>

using namespace fb::sinkline;
using namespace fb::sinkline::operators;

auto test (int value)
{
  auto sink = sinkline(
    map([](int x) {
      return x * 1.5;
    }),
    filter([](double x) {
      return x / 2 > 5.0;
    }),
    scheduleOn(GCDScheduler::mainQueueScheduler()),
    [](double x) {
      printf("%f", x);
    });

  return sink(value);
}
