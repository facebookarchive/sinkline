/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import "SNKImageCell.h"

#import <AVFoundation/AVFoundation.h>

#import "SNKImage.h"
#import "SNKKeyValueObserver.h"

#if USE_SINKLINE
  #import <sinkline/Sinkline.h>

  using namespace fb::sinkline;
  namespace sink = fb::sinkline::operators;
#endif

@interface SNKImageCell ()

@property (nonatomic, strong, readonly) AVPlayerLayer *playerLayer;

@property (nonatomic, strong) SNKKeyValueObserver *keyValueObserver;
@property (nonatomic, strong) id notificationObserver;

@end

@implementation SNKImageCell

#pragma mark Properties

- (void)setImage:(SNKImage *)image
{
  if ([image isEqual:_image]) {
    return;
  }

  [self tearDownPlayer];

  _image = image;
  if (image) {
    [self setUpPlayer];
  }
}

#pragma mark Lifecycle

- (instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
  self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
  if (!self) {
    return nil;
  }

  _playerLayer = [AVPlayerLayer layer];
  _playerLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;

  [self.contentView.layer addSublayer:_playerLayer];

  return self;
}

- (void)dealloc
{
  [self tearDownPlayer];
}

- (void)prepareForReuse
{
  [self tearDownPlayer];
  [super prepareForReuse];
}

- (void)setUpPlayer
{
  auto player = [AVPlayer playerWithURL:self.image.videoURL];
  self.playerLayer.player = player;

  self.keyValueObserver = [[SNKKeyValueObserver alloc] initWithTarget:player keyPath:@"status" changeHandler:^(id value) {
    [player play];
  }];

  #if USE_SINKLINE

  self.notificationObserver = [NSNotificationCenter.defaultCenter addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:player.currentItem queue:nil usingBlock:sinkline(
    sink::then(^(NSNotification *notification, void (^seekCompleted)(BOOL)) {
      [player seekToTime:kCMTimeZero completionHandler:seekCompleted];
    }),
    ^(BOOL finished) {
      if (finished) {
        [player play];
      }
    }
  )];

  #else // #if USE_SINKLINE

  self.notificationObserver = [NSNotificationCenter.defaultCenter addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:player.currentItem queue:nil usingBlock:^(NSNotification *notification) {
    [player seekToTime:kCMTimeZero completionHandler:^(BOOL finished) {
      if (finished) {
        [player play];
      }
    }];
  }];

  #endif // #if USE_SINKLINE
}

- (void)tearDownPlayer
{
  self.keyValueObserver = nil;
  self.notificationObserver = nil;

  self.playerLayer.player = nil;
}

#pragma mark Layout

- (void)layoutSubviews
{
  [super layoutSubviews];

  _playerLayer.frame = self.contentView.bounds;
}

@end
