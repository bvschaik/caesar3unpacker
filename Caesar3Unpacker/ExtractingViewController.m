//
//  ExtractingViewController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "ExtractingViewController.h"

#include <dispatch/dispatch.h>

@interface ExtractingViewController ()
- (void)updateProgressAsync:(NSString *)message;
@end

@implementation ExtractingViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self->shouldCancel = NO;
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
       
        for (int i = 0; i < 10; i++) {
            if (self->shouldCancel) {
                [self updateProgressAsync:@"Cancelled!"];
                break;
            }
            [NSThread sleepForTimeInterval:1];

            [self updateProgressAsync:[NSString stringWithFormat:@"Progress: %d", i]];
        }
    });
}

- (NSString *)title {
    return @"Extracting files";
}

- (void)onCancel:(id)sender {
    self->shouldCancel = YES;
}

- (void)updateProgressAsync:(NSString *)message {
    dispatch_async(dispatch_get_main_queue(), ^{
        self.progressView.string = [self.progressView.string stringByAppendingFormat:@"%@\n", message];
        [self.progressView scrollToEndOfDocument:nil];
        //[self.progressView scrollToEndOfDocument:nil];
    });
}

@end
