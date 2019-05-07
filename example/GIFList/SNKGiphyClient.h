/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import <Foundation/Foundation.h>

@class SNKGIF;

@interface SNKGiphyClient : NSObject

- (instancetype)init;

- (void)fetchNumberOfTrendingGIFs:(NSUInteger)count completionHandler:(void (^)(NSArray<SNKGIF *> *GIFs, NSError *error))completionHandler;

@end
