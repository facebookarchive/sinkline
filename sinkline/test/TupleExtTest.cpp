/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include "TestCommon.h"

#include <sinkline/TupleExt.h>

using namespace fb::sinkline;

TEST(TupleExtTest, Extract)
{
  {
    int value = 0;
    std::tuple<const char *, bool> result = extract(std::make_tuple("foobar", 5, true), &value);
    EXPECT_EQ(value, 5);
    EXPECT_STREQ(std::get<0>(result), "foobar");
    EXPECT_TRUE(std::get<1>(result));
  }

  {
    int value = 0;
    std::tuple<const char *, bool> result = extract(&value, "foobar", 5, true);
    EXPECT_EQ(value, 5);
    EXPECT_STREQ(std::get<0>(result), "foobar");
    EXPECT_TRUE(std::get<1>(result));
  }
}

TEST(TupleExtTest, FlattenOptionals)
{
  {
    auto testTuple = std::make_tuple(Optional<int>(5), Optional<bool>());

    auto flattened = flattenOptionals<0, decltype(testTuple), int, bool>(testTuple);
    EXPECT_FALSE(static_cast<bool>(flattened));
  }

  {
    auto testTuple = std::make_tuple(Optional<int>(5), Optional<bool>(true));

    auto flattened = flattenOptionals<0, decltype(testTuple), int, bool>(testTuple);
    EXPECT_TRUE(static_cast<bool>(flattened));
    EXPECT_EQ(flattened.value(), std::make_tuple(5, true));
  }
}
