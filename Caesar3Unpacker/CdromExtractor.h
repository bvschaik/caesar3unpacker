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

@interface CdromExtractor : NSObject

@property (weak) id<ExtractorDelegate> delegate;

- (void)extract:(WizardState*)state;

@end

NS_ASSUME_NONNULL_END
