//
//  Extractor.h
//  Caesar3Unpacker
//
//  Created by bianca on 02-12-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "ExtractorDelegate.h"
#import "WizardState.h"

NS_ASSUME_NONNULL_BEGIN

@interface Extractor : NSObject {
    WizardState *state;
    ExtractorState result;
}

@property (weak) id<ExtractorDelegate> delegate;

- (Extractor*)initWithState:(WizardState*)state;

- (void)extract;

// protected methods

- (BOOL)doExtract;
- (BOOL)checkForCancel;
- (BOOL)createDirectoryForFile:(NSURL*)fileUrl;

@end


NS_ASSUME_NONNULL_END
