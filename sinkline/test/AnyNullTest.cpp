/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include "TestCommon.h"

#include <sinkline/AnyNull.h>

using namespace fb::sinkline;

TEST(AnyNullTest, AnyNull)
{
  const char *typedNull = nullptr;

  EXPECT_TRUE(anyNull(nullptr));
  EXPECT_TRUE(anyNull(nullptr, "foobar"));
  EXPECT_TRUE(anyNull(typedNull, "foobar"));
  EXPECT_FALSE(anyNull("foobar"));
  EXPECT_FALSE(anyNull("foobar", "foobar"));
  EXPECT_EQ(anyNull(1), IsNullNothingComparable());
  EXPECT_EQ(anyNull(1, 2), IsNullNothingComparable());
  EXPECT_TRUE(anyNull(1, nullptr, 2));
  EXPECT_FALSE(anyNull(1, "foobar", 2));
}
