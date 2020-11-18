//
//  AppController.h
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import "WizardViewController.h"

NS_ASSUME_NONNULL_BEGIN

@interface AppController : NSObject <WizardViewDelegate> {
    WizardPage currentPage;
    WizardState *wizardState;
}

@property (weak) IBOutlet NSTextField *titleLabel;
@property (weak) IBOutlet NSView *wizardView;
@property (weak) IBOutlet NSButton *backButton;
@property (weak) IBOutlet NSButton *nextButton;
@property (strong) WizardViewController *wizardViewController;

- (IBAction)goToNext:(id)sender;
- (IBAction)goBack:(id)sender;

@end

NS_ASSUME_NONNULL_END
