/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import <Foundation/Foundation.h>

@class SNKImage;

@interface SNKGIF : NSObject

- (instancetype)initWithJSON:(NSDictionary *)JSON;

@property (nonatomic, copy, readonly) NSString *ID;
@property (nonatomic, copy, readonly) NSDictionary<NSString *, SNKImage *> *imagesByName;

@end
