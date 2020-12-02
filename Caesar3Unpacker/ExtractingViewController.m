//
//  ExtractingViewController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "ExtractingViewController.h"
#import "CdromExtractor.h"
#import "GogExtractor.h"

#include <dispatch/dispatch.h>

@interface ExtractingViewController ()
- (void)updateProgressAsync:(NSString *)message isFinished:(BOOL)finished;
@end

@implementation ExtractingViewController

- (void)initWizard {
    self.progressView.editable = NO;
    Extractor *extractor;
    if (self.wizardState.sourceId == SourceCdrom) {
        extractor = [[CdromExtractor alloc] initWithState:self.wizardState];
    } else if (self.wizardState.sourceId == SourceGog) {
        extractor = [[GogExtractor alloc] initWithState:self.wizardState];
    } else {
        [self onExtractorError:@"Unknown source selected"];
        return;
    }
    extractor.delegate = self;
    self.wizardState.isCancelled = NO;

    [self.delegate setBackButtonState:ButtonDisabled];
    [self.delegate setNextButtonState:ButtonDisabled];
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
    [self updateProgressAsync:message isFinished:NO];
}

- (void)onExtractorError:(NSString *)message {
    [self updateProgressAsync:message isFinished:NO];
}

- (void)onExtractorDone:(ExtractorState)state {
    NSString *message;
    switch (state) {
        case ExtractorSuccess:
            message = @"Done!";
            break;
        case ExtractorCancelled:
            message = @"*** CANCELLED ***";
            break;
        case ExtractorError:
            message = @"*** ERROR ***";
            break;
    }
    [self updateProgressAsync:message isFinished:YES];
}

- (void)updateProgressAsync:(NSString *)message isFinished:(BOOL)finished {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSTextStorage *textStorage = self.progressView.textStorage;
        [textStorage beginEditing];
        if (textStorage.length > 0) {
            [textStorage replaceCharactersInRange:NSMakeRange(textStorage.length, 0) withString:@"\n"];
        }
        [textStorage replaceCharactersInRange:NSMakeRange(textStorage.length, 0) withString:message];
        [textStorage endEditing];

        [self.progressView scrollToEndOfDocument:nil];

        if (finished) {
            [self.delegate setBackButtonState:ButtonEnabled];
            [self.delegate setNextButtonState:ButtonEnabled];
        }
    });
}

@end
