//
//  WizardViewController.h
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "WizardState.h"

NS_ASSUME_NONNULL_BEGIN

typedef enum {
    WizardNone = 0,
    WizardChooseSource,
    WizardGogSource,
    WizardSteamSource,
    WizardCdromSource,
    WizardTargetDir,
    WizardExtracting
} WizardPage;

@interface WizardViewController : NSViewController

@property (weak) WizardState *wizardState;

- (NSString *)title;
- (WizardPage)getNextPage;
- (WizardPage)getPrevPage;

@end

NS_ASSUME_NONNULL_END
