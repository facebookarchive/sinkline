/**
 * Copyright 2016-present, Facebook, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the license found in the
 * LICENSE-examples file in this source tree.
 */

#import "SNKTrendingViewController.h"

#import "SNKGIF.h"
#import "SNKGiphyClient.h"
#import "SNKImage.h"
#import "SNKImageCell.h"

#if USE_SINKLINE
  #import <sinkline/Scheduler.h>
  #import <sinkline/Sinkline.h>

  using namespace fb::sinkline;
  namespace sink = fb::sinkline::operators;
#endif

static const NSUInteger TrendingViewControllerDefaultTrendingGIFCount = 25;

@interface SNKTrendingViewController () <UITableViewDataSource, UITableViewDelegate>

@property (nonatomic, strong) SNKGiphyClient *client;

@property (atomic, copy) NSArray<SNKGIF *> *GIFs;

@end

@implementation SNKTrendingViewController

#pragma mark Lifecycle

- (void)viewDidLoad
{
  [super viewDidLoad];

  self.client = [SNKGiphyClient new];

  [self.tableView registerClass:SNKImageCell.class forCellReuseIdentifier:NSStringFromClass(SNKImageCell.class)];
}

- (void)viewWillAppear:(BOOL)animated
{
  [super viewWillAppear:animated];

  #if USE_SINKLINE

  [self.client fetchNumberOfTrendingGIFs:TrendingViewControllerDefaultTrendingGIFCount completionHandler:sinkline(
    sink::scheduleOn(GCDScheduler::mainQueueScheduler()),
    sink::onError(^(NSError *error) {
      NSLog(@"Error fetching GIFs: %@", error);

      auto controller = [UIAlertController alertControllerWithTitle:error.localizedDescription message:error.localizedFailureReason preferredStyle:UIAlertControllerStyleAlert];
      auto okAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"OK", nil) style:UIAlertActionStyleDefault handler:^(UIAlertAction *action) {}];
      [controller addAction:okAction];

      [self presentViewController:controller animated:YES completion:nil];
    }),
    sink::sideEffect(^(NSArray<SNKGIF *> *GIFs) {
      NSLog(@"Fetched %zu GIFs", (size_t)GIFs.count);
    }),
    ^(NSArray<SNKGIF *> *GIFs) {
      self.GIFs = GIFs;
      [self.tableView reloadData];
    }
  )];

  #else // #if USE_SINKLINE

  [self.client fetchNumberOfTrendingGIFs:TrendingViewControllerDefaultTrendingGIFCount completionHandler:^(NSArray<SNKGIF *> *GIFs, NSError *error) {
    dispatch_async(dispatch_get_main_queue(), ^{
      if (error) {
        NSLog(@"Error fetching GIFs: %@", error);

        auto controller = [UIAlertController alertControllerWithTitle:error.localizedDescription message:error.localizedFailureReason preferredStyle:UIAlertControllerStyleAlert];
        auto okAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"OK", nil) style:UIAlertActionStyleDefault handler:^(UIAlertAction *action) {}];
        [controller addAction:okAction];

        [self presentViewController:controller animated:YES completion:nil];
        return;
      }

      NSLog(@"Fetched %zu GIFs", (size_t)GIFs.count);
      self.GIFs = GIFs;
      [self.tableView reloadData];
    });
  }];

  #endif // #if USE_SINKLINE
}

#pragma mark UITableViewDataSource

- (SNKImage *)imageForRow:(NSInteger)rowIndex
{
  return self.GIFs[rowIndex].imagesByName[@"original"];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
  return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
  return self.GIFs.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
  SNKImageCell *cell = [tableView dequeueReusableCellWithIdentifier:NSStringFromClass(SNKImageCell.class) forIndexPath:indexPath];

  cell.image = [self imageForRow:indexPath.row];

  return cell;
}

#pragma mark UITableViewDelegate

// TODO: Replace with fixed height rows
- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
  SNKImage *image = [self imageForRow:indexPath.row];
  return image.height;
}

@end
