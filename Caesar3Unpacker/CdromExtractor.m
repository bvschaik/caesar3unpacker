//
//  CdromExtractor.m
//  Caesar3Unpacker
//
//  Created by bianca on 18-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "CdromExtractor.h"

#include "libunshield.h"

@interface CdromExtractor ()
- (BOOL)extractCabFile;
- (BOOL)copyFiles;
@end

@implementation CdromExtractor

- (CdromExtractor *)initWithState:(WizardState *)state {
    return self = [super initWithState:state];
}

- (BOOL)doExtract {
    return [self extractCabFile] && [self copyFiles];
}

- (BOOL)extractCabFile {
    if ([self checkForCancel]) {
        return NO;
    }

    NSURL *dataCab = [state.sourceUrl URLByAppendingPathComponent:@"data1.cab"];
    if (![[NSFileManager defaultManager] fileExistsAtPath:dataCab.path]) {
        [self.delegate onExtractorError:[NSString stringWithFormat:@"File data1.cab was not found in %@. Please check if you have selected the right folder.", state.sourceUrl]];
        return NO;
    }
    Unshield *unshield = unshield_open(dataCab.path.fileSystemRepresentation);
    if (!unshield) {
        [self.delegate onExtractorError:@"File data1.cab is not a valid InstallShield file. Please check if you have selected the right folder."];
        return NO;
    }
    UnshieldFileGroup* file_group = unshield_file_group_find(unshield, "Exe");
    if (!file_group) {
        [self.delegate onExtractorError:@"File data1.cab does not contain an Exe group. Please check if you have selected the right folder."];
        unshield_close(unshield);
        return NO;
    }
    
    for (int i = file_group->first_file; i <= file_group->last_file; i++) {
        if (unshield_file_is_valid(unshield, i)) {
            const char *filename = unshield_file_name(unshield, i);
            [self.delegate onExtractorProgress:[NSString stringWithFormat:@"Extracting file %s", filename]];
            NSURL *path = [state.targetUrl URLByAppendingPathComponent:[NSString stringWithUTF8String:filename]];
            if (![self createDirectoryForFile:path] || !unshield_file_save(unshield, i, path.fileSystemRepresentation)) {
                [self.delegate onExtractorProgress:[NSString stringWithFormat:@"Failed to extract file %s", filename]];
            }

            if ([self checkForCancel]) {
                break;
            }
        }
    }
    unshield_close(unshield);
    return !state.isCancelled;
}

- (BOOL)copyFiles {
    [self.delegate onExtractorProgress:@"Copying files"];

    BOOL success = YES;
    success &= [self copyDirectory:[state.sourceUrl URLByAppendingPathComponent:@"555"] to:[state.targetUrl URLByAppendingPathComponent:@"555"]];
    success &= [self copyDirectory:[state.sourceUrl URLByAppendingPathComponent:@"SMK"] to:[state.targetUrl URLByAppendingPathComponent:@"SMK"]];
    NSURL *targetWavs = [state.targetUrl URLByAppendingPathComponent:@"wavs"];
    success &= [self copyDirectory:[state.sourceUrl URLByAppendingPathComponent:@"wavs"] to:targetWavs];
    success &= [self copyDirectory:[state.sourceUrl URLByAppendingPathComponent:@"Soundfx"] to:targetWavs];

    return success;
}

- (BOOL)copyDirectory:(NSURL *)from to:(NSURL *)to {
    if ([self checkForCancel]) {
        return NO;
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];

    if (![fileManager fileExistsAtPath:from.path]) {
        [self.delegate onExtractorProgress:[NSString stringWithFormat:@"Expected folder does not exist: %@", from.path]];
        return YES; // Not a fatal error, some versions don't include all files
    }

    NSError *error;
    if (![fileManager createDirectoryAtURL:to withIntermediateDirectories:YES attributes:nil error:&error]) {
        [self.delegate onExtractorError:[NSString stringWithFormat:@"Unable to create directory %@: %@", to.path, error.localizedDescription]];
        return NO;
    }

    NSArray<NSURL*> *files = [fileManager contentsOfDirectoryAtURL:from includingPropertiesForKeys:nil options:0 error:nil];
    for (NSURL *file in files) {
        [fileManager copyItemAtURL:file toURL:[to URLByAppendingPathComponent:file.lastPathComponent] error:nil];
        [self.delegate onExtractorProgress:[NSString stringWithFormat:@"Copying file %@", file.lastPathComponent]];

        if ([self checkForCancel]) {
            break;
        }
    }
    return YES;
}

@end
