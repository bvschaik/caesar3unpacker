//
//  GogExtractor.h
//  Caesar3Unpacker
//
//  Created by bianca on 02-12-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "Extractor.h"

NS_ASSUME_NONNULL_BEGIN

@interface GogExtractor : Extractor

- (GogExtractor*)initWithState:(WizardState*)state;

@end

NS_ASSUME_NONNULL_END
