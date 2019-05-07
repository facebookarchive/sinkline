/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include <sinkline/Sinkline.h>

#include <tuple>

using namespace fb::sinkline;
using namespace fb::sinkline::operators;

static int transform (int input)
{
  return input;
}

auto test (std::tuple<int> tuple)
{
  auto sink = reduce().compose(&transform);
  return sink(tuple);
}
