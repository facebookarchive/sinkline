/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#include "TestCommon.h"

#include <sinkline/Either.h>
#include <sinkline/OperatorDefinitions.h>
#include <sinkline/Operators.h>
#include <sinkline/Scheduler.h>
#include <sinkline/Sinkline.h>

#include <unistd.h>

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

using namespace fb::sinkline;
using namespace fb::sinkline::operators;

TEST(OperatorsTest, Map)
{
  auto mapSink = map([](int value) {
    return value * 2;
  }).compose([](int value) {
    return std::to_string(value);
  });

  EXPECT_EQ(mapSink(0), "0");
  EXPECT_EQ(mapSink(1), "2");
  EXPECT_EQ(mapSink(2), "4");
  EXPECT_EQ(mapSink(21), "42");
}

TEST(OperatorsTest, Filter)
{
  auto filterSink = filter([](int value) {
    return value % 2 == 0;
  }).compose([](int value) {
    return std::to_string(value);
  });

  EXPECT_TRUE(bool(filterSink(0)));
  EXPECT_TRUE(bool(filterSink(2)));
  EXPECT_TRUE(bool(filterSink(4)));

  EXPECT_EQ(filterSink(0).value(), "0");
  EXPECT_EQ(filterSink(2).value(), "2");
  EXPECT_EQ(filterSink(4).value(), "4");

  EXPECT_FALSE(bool(filterSink(1)));
  EXPECT_FALSE(bool(filterSink(3)));
  EXPECT_FALSE(bool(filterSink(5)));
}

TEST(OperatorsTest, Scan)
{
  auto scanSink = scan(1, [](int accumulated, int value) {
    return accumulated + value;
  }).compose([](int value) {
    return std::to_string(value);
  });

  EXPECT_EQ(scanSink(0), "1");
  EXPECT_EQ(scanSink(1), "2");
  EXPECT_EQ(scanSink(2), "4");
  EXPECT_EQ(scanSink(3), "7");
}

TEST(OperatorsTest, ScanUnlocked)
{
  auto scanSink = scan<void>(1, [](int accumulated, int value) {
    return accumulated + value;
  }).compose([](int value) {
    return std::to_string(value);
  });

  EXPECT_EQ(scanSink(0), "1");
  EXPECT_EQ(scanSink(1), "2");
  EXPECT_EQ(scanSink(2), "4");
  EXPECT_EQ(scanSink(3), "7");
}

TEST(OperatorsTest, IgnoreNull)
{
  auto ignoreNullSink = ignoreNull().compose([](const char *str) {
    return std::string(str);
  });

  EXPECT_FALSE(bool(ignoreNullSink(nullptr)));
  EXPECT_EQ(ignoreNullSink("foobar").value(), "foobar");
}

TEST(OperatorsTest, Combine)
{
  CombineOperator<std::string, int, int> sumSink([](int a, int b) {
    return std::to_string(a + b);
  });

  EXPECT_FALSE(bool(std::get<0>(sumSink.sinks())(1)));
  EXPECT_EQ(std::get<1>(sumSink.sinks())(2).value(), "3");
  EXPECT_EQ(std::get<1>(sumSink.sinks())(4).value(), "5");
  EXPECT_EQ(std::get<0>(sumSink.sinks())(5).value(), "9");
}

TEST(OperatorsTest, ScheduleOn)
{
  auto schedulingSink = scheduleOn(ImmediateScheduler()).compose([](int value) {
    return value + 1;
  });

  EXPECT_EQ(schedulingSink(0).get(), 1);
  EXPECT_EQ(schedulingSink(1).get(), 2);
  EXPECT_EQ(schedulingSink(4).get(), 5);
}

TEST(OperatorsTest, SideEffect)
{
  int sum = 0;

  auto sideEffectSink = sideEffect([&sum](int value) {
    sum += value;
  }).compose([](int value) {
    return std::to_string(value);
  });

  EXPECT_EQ(sideEffectSink(0), "0");
  EXPECT_EQ(sideEffectSink(1), "1");
  EXPECT_EQ(sideEffectSink(2), "2");
  EXPECT_EQ(sideEffectSink(3), "3");
  EXPECT_EQ(sideEffectSink(4), "4");

  EXPECT_EQ(sum, 10);
}

TEST(OperatorsTest, OnError)
{
  int errors = 0;
  int successes = 0;

  auto errorSink = onError([&errors](const char *error) {
    EXPECT_STREQ(error, "foobar");
    errors++;
  }).compose([&successes](int x) {
    EXPECT_EQ(x, 5);
    successes++;
  });

  errorSink("foobar", 5);
  EXPECT_EQ(successes, 0);
  EXPECT_EQ(errors, 1);

  // TODO: Remove need for this cast.
  errorSink(static_cast<const char *>(nullptr), 5);
  EXPECT_EQ(successes, 1);
  EXPECT_EQ(errors, 1);
}

TEST(OperatorsTest, RecoverSink)
{
  int errors = 0;
  int successes = 0;

  auto recoverSink = recover([&errors](const char *error) {
    EXPECT_STREQ(error, "foobar");
    errors++;

    return 5;
  }).compose([&successes](int x) {
    EXPECT_EQ(x, 5);
    successes++;
  });

  recoverSink("foobar", 5);
  EXPECT_EQ(successes, 1);
  EXPECT_EQ(errors, 1);

  // TODO: Remove need for this cast.
  recoverSink(static_cast<const char *>(nullptr), 5);
  EXPECT_EQ(successes, 2);
  EXPECT_EQ(errors, 1);
}

TEST(OperatorsTest, Then)
{
  auto addSink = [](int a, int b) {
    return a + b;
  };

  auto thenSink = then([](const char *input, decltype(addSink) next) {
    auto length = static_cast<int>(strlen(input));
    return next(length, length * 2) * 1.5;
  }).compose(addSink);

  auto result = thenSink("foo");
  auto expected = (3 + 3 * 2) * 1.5;
  EXPECT_LT(fabs(result - expected), 0.01);
}

TEST(OperatorsTest, SinklineIfWithCondition)
{
  int sum = 0;

  auto sink = [&sum](int value) {
    sum += value;
    return sum;
  };

  auto enabled = sinklineIf(true, sink);
  EXPECT_EQ(enabled(1).value(), 1);
  EXPECT_EQ(enabled(2).value(), 3);
  EXPECT_EQ(sum, 3);

  auto disabled = sinklineIf(false, sink);
  EXPECT_FALSE(disabled(5));
  EXPECT_EQ(sum, 3);
}

TEST(OperatorsTest, SinklineIfWithoutCondition)
{
  int sum = 0;

  std::function<int(int)> sink = [&sum](int value) {
    sum += value;
    return sum;
  };

  auto enabled = sinklineIf(sink);
  EXPECT_EQ(enabled(1).value(), 1);
  EXPECT_EQ(enabled(2).value(), 3);
  EXPECT_EQ(sum, 3);

  sink = nullptr;

  auto disabled = sinklineIf(sink);
  EXPECT_FALSE(disabled(5));
  EXPECT_EQ(sum, 3);
}

static void asynchronousAdder(std::shared_ptr<ThreadScheduler> scheduler, int start, int end, std::function<void(int)> callback)
{
  // HACK until we fix up our tests to terminate normally:
  // https://github.com/facebook/sinkline/issues/43
  if (start == end) {
    return;
  }

  scheduler->schedule([=] {
    int newValue = start + 1;

    callback(start + 1);
    asynchronousAdder(scheduler, newValue, end, callback);
  });
}

static void asynchronousAdder (std::shared_ptr<ThreadScheduler> scheduler, int start, std::function<void(int)> callback)
{
  asynchronousAdder(scheduler, start, start + 10, callback);
}

TEST(OperatorsTest, Sinklines)
{
  auto scheduler = std::make_shared<ThreadScheduler>();

  {
    auto callback = [](int a, int b, int c) {
      std::cout << "a = " << a << ", b = " << b << ", c = " << c << std::endl;
    };

    CombineOperator<void, int, int, int> sink(callback);
    auto sinks = sink.sinks();

    asynchronousAdder(scheduler, 0, std::get<0>(sinks));
    asynchronousAdder(scheduler, 0, std::get<1>(sinks));
    asynchronousAdder(scheduler, 0, std::get<2>(sinks));
  }

  {
    asynchronousAdder(scheduler, 23456, sinkline(
      map([](double value) {
        return static_cast<int>(value * 1.5);
      }),
      filter([](int value) {
        return value % 2 == 0;
      }),
      scan(0, [](int accumulated, int value) {
        return accumulated + value;
      }),
      scheduleOn(ThreadScheduler()),
      [](int value) {
        std::cout << "INT VALUE: " << value << std::endl;
      }));
  }

  {
    std::function<void(double, int)> callback = sinkline(
      onError([](int value) {
        std::cout << "CAUGHT INTEGER: " << value << std::endl;
      }),
      [](double value) {
        std::cout << "SUCCESSFUL DOUBLE: " << value << std::endl;
      });

    CombineOperator<void, double, int> sink(callback);
    auto sinks = sink.sinks();

    auto mapToDouble = sinkline(
      map([](int value) {
        return value * 1.5;
      }),
      std::get<0>(sinks));

    asynchronousAdder(scheduler, 10000, mapToDouble);
    asynchronousAdder(scheduler, 1000, std::get<1>(sinks));
  }

  {
    auto sink = sinkline(
      map([](Either<int, double> value) {
        return value.match(
          [](int x) { return x * 2; },
          [](double x) { return static_cast<int>(x * 2); }
        );
      }),
      [](int value) {
        std::cout << "UNPACKED EITHER VALUE: " << value << std::endl;
      });

    Either<int, double> (*leftMap)(int) = &makeLeft<int, double>;
    Either<int, double> (*rightMap)(double) = &makeRight<int, double>;

    auto intSink = sinkline(
      map(leftMap),
      sink
    );

    auto doubleSink = sinkline(
      map(rightMap),
      sink
    );

    intSink(1);
    intSink(2);

    doubleSink(1.5);
    doubleSink(2.5);
  }

  sleep(1);
}
