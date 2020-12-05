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
- (NSURL*)getFile:(NSString*)filename inDirectory:(NSURL*)directory;
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

    NSURL *dataCab = [self getFile:@"data1.cab" inDirectory:state.sourceUrl];
    if (!dataCab) {
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
    success &= [self copyDirectory:[self getFile:@"555" inDirectory:state.sourceUrl] to:[state.targetUrl URLByAppendingPathComponent:@"555"]];
    success &= [self copyDirectory:[self getFile:@"SMK" inDirectory:state.sourceUrl] to:[state.targetUrl URLByAppendingPathComponent:@"SMK"]];
    NSURL *targetWavs = [state.targetUrl URLByAppendingPathComponent:@"wavs"];
    success &= [self copyDirectory:[self getFile:@"wavs" inDirectory:state.sourceUrl] to:targetWavs];
    success &= [self copyDirectory:[self getFile:@"Soundfx" inDirectory:state.sourceUrl] to:targetWavs];

    return success;
}

- (BOOL)copyDirectory:(NSURL *)from to:(NSURL *)to {
    if ([self checkForCancel]) {
        return NO;
    }

    if (!from) {
        return YES; // Not a fatal error, some versions don't include all files
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];

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

- (NSURL *)getFile:(NSString *)filename inDirectory:(NSURL *)directory {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *defaultUrl = [directory URLByAppendingPathComponent:filename];
    if ([fileManager fileExistsAtPath:defaultUrl.path]) {
        return defaultUrl;
    }
    NSArray<NSURL*> *files = [fileManager contentsOfDirectoryAtURL:directory includingPropertiesForKeys:nil options:0 error:nil];
    for (NSURL *file in files) {
        if ([filename caseInsensitiveCompare:file.lastPathComponent] == NSOrderedSame) {
            return file;
        }
    }
    return nil;
}

@end
