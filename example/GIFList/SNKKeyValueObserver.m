/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import "SNKKeyValueObserver.h"

static void *SNKKeyValueObserverContext = &SNKKeyValueObserverContext;

@interface SNKKeyValueObserver ()

@property (nonatomic, strong, readonly) NSObject *target;
@property (nonatomic, copy, readonly) NSString *keyPath;
@property (nonatomic, copy, readonly) void (^changeHandler)(id);

@end

@implementation SNKKeyValueObserver

#pragma mark Lifecycle

- (instancetype)initWithTarget:(NSObject *)target keyPath:(NSString *)keyPath changeHandler:(void (^)(id newValue))changeHandler
{
  NSParameterAssert(target != nil);
  NSParameterAssert(keyPath != nil);
  NSParameterAssert(changeHandler != nil);

  self = [super init];
  if (!self) {
    return nil;
  }

  _target = target;
  _keyPath = [keyPath copy];
  _changeHandler = [changeHandler copy];

  [_target addObserver:self forKeyPath:_keyPath options:NSKeyValueObservingOptionInitial context:SNKKeyValueObserverContext];

  return self;
}

- (void)dealloc
{
  [_target removeObserver:self forKeyPath:_keyPath context:SNKKeyValueObserverContext];
}

#pragma mark NSKeyValueObserving

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
  if (context != SNKKeyValueObserverContext) {
    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    return;
  }

  self.changeHandler([object valueForKeyPath:self.keyPath]);
}

@end
