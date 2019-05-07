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

static bool predicate (int input)
{
  return input != 0;
}

auto test (int value)
{
  auto sink = filter(&predicate).compose(&predicate);
  return sink(value);
}
