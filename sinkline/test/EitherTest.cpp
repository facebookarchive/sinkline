/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include "TestCommon.h"

#include <sinkline/Either.h>

using namespace fb::sinkline;

TEST(EitherTest, Either)
{
  Either<int, bool> e(5);

  auto leftMatch = [](int value) { return value * 2; };
  auto rightMatch = [](bool value) { return 0; };

  EXPECT_TRUE(e.hasLeft());
  EXPECT_FALSE(e.hasRight());

  EXPECT_EQ(e.left(), 5);
  EXPECT_EQ(e.match(leftMatch, rightMatch), 10);

  e = makeRight<int, bool>(false);

  EXPECT_FALSE(e.hasLeft());
  EXPECT_TRUE(e.hasRight());

  EXPECT_FALSE(e.right());
  EXPECT_EQ(e.match(leftMatch, rightMatch), 0);
}
