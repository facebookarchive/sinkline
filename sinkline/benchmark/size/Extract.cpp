/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include <sinkline/TupleExt.h>

using namespace fb::sinkline;

auto test (int value)
{
  return extract<int>(nullptr, value);
}
