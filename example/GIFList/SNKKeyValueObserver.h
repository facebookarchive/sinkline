/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import <Foundation/Foundation.h>

// Real code should consider using https://github.com/facebook/KVOController
// instead!
@interface SNKKeyValueObserver : NSObject

- (instancetype)initWithTarget:(NSObject *)target keyPath:(NSString *)keyPath changeHandler:(void (^)(id newValue))changeHandler;

@end
