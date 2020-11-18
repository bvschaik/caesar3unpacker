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

typedef enum {
    ButtonDisabled,
    ButtonEnabled,
    ButtonHidden
} ButtonState;

@protocol WizardViewDelegate

- (void)setBackButtonState:(ButtonState)state;
- (void)setNextButtonState:(ButtonState)state;

@end


@interface WizardViewController : NSViewController

@property (weak) WizardState *wizardState;
@property (nonatomic,weak) id<WizardViewDelegate> delegate;

- (void)initWizard;
- (NSString *)title;
- (WizardPage)getNextPage;
- (WizardPage)getPrevPage;

@end

NS_ASSUME_NONNULL_END
