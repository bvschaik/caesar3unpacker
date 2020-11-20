//
//  CdromExtractor.h
//  Caesar3Unpacker
//
//  Created by bianca on 18-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "ExtractorDelegate.h"
#import "WizardState.h"

NS_ASSUME_NONNULL_BEGIN

@interface CdromExtractor : NSObject {
    WizardState *state;
    ExtractorState result;
}

@property (weak) id<ExtractorDelegate> delegate;

- (CdromExtractor*)initWithState:(WizardState*)state;

- (void)extract;

@end

NS_ASSUME_NONNULL_END
