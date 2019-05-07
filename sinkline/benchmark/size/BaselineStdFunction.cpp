/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include <functional>

static int transform (int input)
{
  return input;
}

auto test (int value)
{
  std::function<int(int)> fn(&transform);
  return fn(value);
}
