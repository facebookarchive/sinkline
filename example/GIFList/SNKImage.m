/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import "SNKImage.h"

@implementation SNKImage

#pragma mark - Lifecycle

- (instancetype)initWithJSON:(NSDictionary *)JSON
{
  NSParameterAssert(JSON);

  self = [super init];
  if (!self) {
    return nil;
  }

  _width = (NSUInteger)[JSON[@"width"] longLongValue];
  _height = (NSUInteger)[JSON[@"height"] longLongValue];

  NSString *URLString = JSON[@"mp4"];
  if (!_width || !_height || !URLString) {
    return nil;
  }

  _videoURL = [NSURL URLWithString:URLString];

  return self;
}

#pragma mark - NSObject

- (NSUInteger)hash
{
  return self.videoURL.hash;
}

- (BOOL)isEqual:(SNKImage *)image
{
  if (self == image) {
    return YES;
  }
  
  if (![image isKindOfClass:SNKImage.class]) {
    return NO;
  }

  return self.width == image.width && self.height == image.height && [self.videoURL isEqual:image.videoURL];
}

- (NSString *)description
{
  return [NSString stringWithFormat:@"<%@: %p>{ width = %zu, height = %zu, videoURL = %@ }", self.class, self, (size_t)self.width, (size_t)self.height, self.videoURL];
}

@end
