/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import "SNKGIF.h"

#import "SNKImage.h"

@implementation SNKGIF

- (instancetype)initWithJSON:(NSDictionary *)JSON
{
  NSParameterAssert(JSON);

  self = [super init];
  if (!self) {
    return nil;
  }

  _ID = JSON[@"id"];
  if (!_ID) {
    return nil;
  }

  NSDictionary *unparsedImages = JSON[@"images"];
  NSMutableDictionary *imagesByName = [NSMutableDictionary dictionary];

  [unparsedImages enumerateKeysAndObjectsUsingBlock:^(NSString *imageName, NSDictionary *imageDictionary, BOOL *stop) {
    SNKImage *image = [[SNKImage alloc] initWithJSON:imageDictionary];
    if (image) {
      imagesByName[imageName] = image;
    }
  }];

  _imagesByName = imagesByName;
  return self;
}

#pragma mark - NSObject

- (NSUInteger)hash
{
  return self.ID.hash;
}

- (BOOL)isEqual:(SNKGIF *)GIF
{
  if (self == GIF) {
    return YES;
  }
  
  if (![GIF isKindOfClass:SNKGIF.class]) {
    return NO;
  }

  return [self.ID isEqual:GIF.ID];
}

- (NSString *)description
{
  return [NSString stringWithFormat:@"<%@: %p>{ ID = %@ } images:\n%@", self.class, self, self.ID, self.imagesByName];
}

@end
