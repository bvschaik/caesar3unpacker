//
//  WizardState.h
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef enum {
    SourceNone = 0,
    SourceGog = 1,
    SourceSteam = 2,
    SourceCdrom = 3
} Source;

@interface WizardState : NSObject

@property Source sourceId;
@property NSString *sourcePath;
@property NSString *targetPath;

@end

NS_ASSUME_NONNULL_END
