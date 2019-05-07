/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import "SNKGiphyClient.h"

#import "SNKGIF.h"

#if USE_SINKLINE
  #import <sinkline/Sinkline.h>

  using namespace fb::sinkline;
  namespace sink = fb::sinkline::operators;
#endif

@interface SNKGiphyClient ()

@property (nonatomic, copy, readonly) NSString *APIKey;

@end

@implementation SNKGiphyClient

#pragma mark Lifecycle

- (instancetype)init
{
  self = [super init];
  if (!self) {
    return nil;
  }

  // This is the "public beta key" from:
  // https://github.com/Giphy/GiphyAPI
  _APIKey = @"dc6zaTOxFJmzC";

  return self;
}

#pragma mark API calls

- (NSURL *)URLForRequestingEndpoint:(NSString *)endpoint queryItems:(NSArray<NSURLQueryItem *> *)queryItems
{
  NSURLComponents *components = [[NSURLComponents alloc] initWithString:@"http://api.giphy.com"];
  components.path = [@"/v1/gifs/" stringByAppendingPathComponent:endpoint];

  NSURLQueryItem *keyItem = [NSURLQueryItem queryItemWithName:@"api_key" value:self.APIKey];
  components.queryItems = [queryItems arrayByAddingObject:keyItem];

  return components.URL;
}

- (void)fetchNumberOfTrendingGIFs:(NSUInteger)count completionHandler:(void (^)(NSArray<SNKGIF *> *GIFs, NSError *error))completionHandler
{
  NSParameterAssert(count > 0);

  NSURLQueryItem *limit = [NSURLQueryItem queryItemWithName:@"limit" value:[NSString stringWithFormat:@"%zu", (size_t)count]];
  NSURL *requestURL = [self URLForRequestingEndpoint:@"trending" queryItems:@[ limit ]];

  #if USE_SINKLINE

  auto task = [NSURLSession.sharedSession dataTaskWithURL:requestURL completionHandler:sinkline(
    sink::map(^(NSData *data, NSURLResponse *response, NSError *error) {
      NSMutableArray<SNKGIF *> *GIFs = nil;

      if (data) {
        NSDictionary *result = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];
        NSArray *items = result[@"data"];

        if (items) {
          GIFs = [NSMutableArray arrayWithCapacity:items.count];

          for (NSDictionary *item in items) {
            auto GIF = [[SNKGIF alloc] initWithJSON:item];
            if (GIF) {
              [GIFs addObject:GIF];
            }
          }
        }
      }

      return std::make_tuple((NSArray<SNKGIF *> *)GIFs, error);
    }),
    sink::reduce(),
    sinklineIf(completionHandler, completionHandler)
  )];

  #else // #if USE_SINKLINE

  auto task = [NSURLSession.sharedSession dataTaskWithURL:requestURL completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
    if (!data) {
      if (completionHandler) {
        completionHandler(nil, error);
      }

      return;
    }

    NSDictionary *result = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];
    NSArray *items = result[@"data"];
    if (!items) {
      if (completionHandler) {
        completionHandler(nil, error);
      }

      return;
    }

    NSMutableArray<SNKGIF *> *GIFs = [NSMutableArray arrayWithCapacity:items.count];

    for (NSDictionary *item in items) {
      auto GIF = [[SNKGIF alloc] initWithJSON:item];
      if (GIF) {
        [GIFs addObject:GIF];
      }
    }

    if (completionHandler) {
      completionHandler(GIFs, nil);
    }
  }];

  #endif // #if USE_SINKLINE
  
  [task resume];
}

@end
