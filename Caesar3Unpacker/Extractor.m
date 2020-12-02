//
//  Extractor.m
//  Caesar3Unpacker
//
//  Created by bianca on 02-12-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "Extractor.h"

@implementation Extractor

- (Extractor *)initWithState:(WizardState *)state {
    self = [super init];
    if (self) {
        self->state = state;
        self->result = ExtractorError;
    }
    return self;
}

- (void)extract {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error;
    if (![fileManager createDirectoryAtURL:state.targetUrl withIntermediateDirectories:YES attributes:nil error:&error]) {
        [self.delegate onExtractorError:[NSString stringWithFormat:@"Unable to create directory %@: %@", state.targetUrl.path, error.localizedDescription]];
        [self.delegate onExtractorDone:ExtractorError];
        return;
    }

    if ([self doExtract] && result != ExtractorCancelled) {
        result = ExtractorSuccess;
    }
    [self.delegate onExtractorDone:result];
}

- (BOOL)doExtract {
    return NO;
}

- (BOOL)checkForCancel {
    if (state.isCancelled) {
        result = ExtractorCancelled;
        return YES;
    }
    return NO;
}

- (BOOL)createDirectoryForFile:(NSURL *)fileUrl {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error;
    NSURL *directory = [fileUrl URLByDeletingLastPathComponent];
    if ([fileManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:&error]) {
        return YES;
    } else {
        [self.delegate onExtractorError:[NSString stringWithFormat:@"Unable to create directory %@: %@", directory.path, error.localizedDescription]];
        return NO;
    }
}

@end
