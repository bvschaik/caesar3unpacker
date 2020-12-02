//
//  GogExtractor.m
//  Caesar3Unpacker
//
//  Created by bianca on 02-12-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "GogExtractor.h"

#include "gogextract/gogextract.h"

@interface GogExtractor ()
- (BOOL)createIntermediateDirectories;
@end

@implementation GogExtractor

- (GogExtractor *)initWithState:(WizardState *)state {
    return self = [super initWithState:state];
}

- (BOOL)doExtract {
    if ([self checkForCancel]) {
        return NO;
    }
    gogextract *g = gogextract_open(state.sourceUrl.fileSystemRepresentation);
    if (!g) {
        [self.delegate onExtractorError:[NSString stringWithFormat:@"Unable to open file: %s", gogextract_error()]];
        return NO;
    }

    int numFiles = gogextract_get_num_files(g);
    for (int i = 0; i < numFiles; i++) {
        if ([self checkForCancel]) {
            break;
        }

        const char *filename = gogextract_get_file_name(g, i);
        [self.delegate onExtractorProgress:[NSString stringWithFormat:@"Extracting file %s", filename]];
        NSURL *path = [state.targetUrl URLByAppendingPathComponent:[NSString stringWithUTF8String:filename]];
        if (![self createDirectoryForFile:path] || !gogextract_save_file(g, i, path.fileSystemRepresentation)) {
            [self.delegate onExtractorProgress:[NSString stringWithFormat:@"Failed to extract file %s", filename]];
        }
    }
    gogextract_close(g);
    return YES;
}

@end
