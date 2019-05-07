# Sinkline [![Build Status](https://travis-ci.com/facebook/sinkline.svg?token=MEsrbrVDhdobySepy2pc&branch=master)](https://travis-ci.com/facebook/sinkline)

**Sinkline** is a C++14 library for structuring callbacks in a declarative, composable way, inspired by [functional reactive programming][] (FRP), [Reactive Extensions][] (Rx), and [ReactiveCocoa][] (RAC).

Sinkline’s fundamental unit of abstraction is the **sink**. Sinks are used just like normal callbacks (e.g., lambdas or functions), but can be inspected and composed in interesting ways, using functional primitives like `map` and `filter`.

## Design goals

Sinkline is designed with the following goals in mind, in no particular order.

Although it is sometimes impossible to satisfy all of these goals at once, the _principles_ underlying the goals are the most important, and all changes to the library should be made with them in mind.

#### Easy to get started

It should be easy to “get your feet wet” with Sinkline, and start using it immediately without having to read a bunch of accompanying documentation or literature.

This means that the library should **be accessible** and **offer incremental benefits**. It should be possible to start using it within an existing codebase—even within just one function—and see some benefit.

#### Easy to reason about

One of the problems with `Observable`s in [Rx][Reactive Extensions] (and `SignalProducer`s in [ReactiveCocoa][]) is that they make it very difficult to reason about effects. Does subscribing to an `Observable` start work each time, just once, or never? Are values buffered, multicasted, or neither?

Unfortunately, it is very difficult for any library to definitively answer those questions. Instead, Sinkline should be a **transparent abstraction** that doesn't make it harder to reason about the underlying code.

#### Works transparently with existing code

Existing code should not have to be refactored to “speak in the language” that Sinkline uses.

Instead, the library should offer **transparent interoperability** with other methods of asynchronous programming. For example, it should be possible to use sinks instead of callbacks _without_ needing to refactor the callback-based APIs.

#### Abstracts away concurrency and state

Concurrency and state are extremely difficult to manage correctly, so the library should take responsibility for both whenever possible.

Practically, this might mean **handling synchronization automatically** "behind the scenes," or offering ways to replace stateful variables with **functional primitives** (e.g. `scan` instead of a stateful accumulator).

#### “Zero overhead”

The sink abstractions should not impose a performance penalty.

Although true “zero overhead” abstraction is more-or-less impossible, the library should do as much as possible to eliminate overhead. Ideally, sink-based code should **match the performance of imperative equivalents** (though not necessarily imperative code that has been extensively optimized by hand).

## Getting started

Sinkline uses [Buck][] to build. Once you have [installed Buck][Downloading and Installing Buck], you can build the library like so:

```sh
buck build //sinkline
```

To run the tests:

```sh
buck test //sinkline
```

There is also an [example iOS application][GIFList] which uses Sinkline. To build and run it in the iOS simulator:

```sh
buck install -r //example:GIFList#iphonesimulator-x86_64
```

### GCC instructions

The [above instructions](#getting-started) assume the [Clang][] compiler. To use GCC instead, you must specify a configuration file:

```sh
buck build --config //buildfile.includes=//script/gcc_defs.py
```

This same `--config` flag can also be passed to `buck test`.

## License

Sinkline is [MIT licensed][LICENSE].

[Buck]: https://buckbuild.com
[Clang]: http://clang.llvm.org
[Downloading and Installing Buck]: https://buckbuild.com/setup/install.html
[functional reactive programming]: https://en.wikipedia.org/wiki/Functional_reactive_programming
[GIFList]: example/GIFList/
[LICENSE]: LICENSE
[Reactive Extensions]: https://rx.codeplex.com
[ReactiveCocoa]: https://github.com/ReactiveCocoa/ReactiveCocoa
