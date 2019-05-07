/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include <sinkline/Sinkline.h>

using namespace fb::sinkline;
using namespace fb::sinkline::operators;

static int transform (int input)
{
  return input;
}

static double transform2 (int input)
{
  return input;
}

auto test (int value)
{
  auto sink = sinkline(
    map(&transform),
    map(&transform2),
    [](double x) {
      return x;
    });

  return sink(value);
}
