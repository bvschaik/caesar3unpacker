//
//  CdromExtractor.h
//  Caesar3Unpacker
//
//  Created by bianca on 18-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "Extractor.h"

NS_ASSUME_NONNULL_BEGIN

@interface CdromExtractor : Extractor

- (CdromExtractor*)initWithState:(WizardState*)state;

@end

NS_ASSUME_NONNULL_END
