//
//  ExtractingViewController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "ExtractingViewController.h"
#import "CdromExtractor.h"

#include <dispatch/dispatch.h>

@interface ExtractingViewController ()
- (void)updateProgressAsync:(NSString *)message;
@end

@implementation ExtractingViewController

- (void)initWizard {
    CdromExtractor *extractor = [[CdromExtractor alloc] initWithState:self.wizardState];
    extractor.delegate = self;
    self.wizardState.isCancelled = NO;
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0), ^{
        [extractor extract];
    });
}

- (NSString *)title {
    return @"Extracting files";
}

- (void)onCancel:(id)sender {
    self.wizardState.isCancelled = YES;
}

- (void)onExtractorProgress:(NSString *)message {
    [self updateProgressAsync:message];
}

- (void)onExtractorError:(NSString *)message {
    [self updateProgressAsync:message];
    [self updateProgressAsync:@"*** ERROR ***"];
}

- (void)onExtractorDone {
    [self updateProgressAsync:@"Done!"];
}

- (void)updateProgressAsync:(NSString *)message {
    dispatch_async(dispatch_get_main_queue(), ^{
        self.progressView.string = [self.progressView.string stringByAppendingFormat:@"%@\n", message];
        [self.progressView scrollToEndOfDocument:nil];
    });
}

@end
