/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include <sinkline/OperatorDefinitions.h>
#include <sinkline/Sinkline.h>

using namespace fb::sinkline;
using namespace fb::sinkline::operators;

static double transform (int x, double y)
{
  return x + y;
}

auto test (int value)
{
  CombineOperator<double, int, double> sink(&transform);

  std::get<0>(sink.sinks())(value);
  return std::get<1>(sink.sinks())(value);
}
