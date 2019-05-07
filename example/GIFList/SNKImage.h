/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import <Foundation/Foundation.h>

@interface SNKImage : NSObject

- (instancetype)initWithJSON:(NSDictionary *)JSON;

@property (nonatomic, assign, readonly) NSUInteger width;
@property (nonatomic, assign, readonly) NSUInteger height;
@property (nonatomic, copy, readonly) NSURL *videoURL;

@end
