//
//  AppController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "AppController.h"
#import "WizardViewController.h"

#include "CdromSourceViewController.h"
#include "ChooseSourceViewController.h"
#include "ExtractingViewController.h"
#include "GogSourceViewController.h"
#include "SteamSourceViewController.h"
#include "TargetDirViewController.h"

@interface AppController()
- (void)goToPage:(NSInteger)pageId;
- (WizardViewController*)createViewController:(NSInteger)pageId;
@end

@implementation AppController

- (instancetype)init {
    wizardState = [[WizardState alloc] init];
    return self;
}

- (void)awakeFromNib {
    [self goToPage:WizardChooseSource];
}

- (void)setBackButtonState:(ButtonState)state {
    self.backButton.hidden = state == ButtonHidden ? YES : NO;
    self.backButton.enabled = state == ButtonEnabled ? YES : NO;
}

- (void)setNextButtonState:(ButtonState)state {
    self.nextButton.hidden = state == ButtonHidden ? YES : NO;
    self.nextButton.enabled = state == ButtonEnabled ? YES : NO;
}

- (void)goToNext:(id)sender {
    NSInteger nextPage = [_wizardViewController getNextPage];
    if (nextPage) {
        [self goToPage:nextPage];
    }
}

- (void)goBack:(id)sender {
    NSInteger prevPage = [_wizardViewController getPrevPage];
    if (prevPage) {
        [self goToPage:prevPage];
    }
}

- (void)goToPage:(NSInteger)pageId {
    WizardViewController *controller = [self createViewController:pageId];
    if (controller) {
        [_wizardViewController.view removeFromSuperview];
        [_wizardView addSubview:controller.view];
        [controller.view setFrame:[_wizardView bounds]];
        [controller.view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

        [self setNextButtonState:ButtonEnabled];
        [self setBackButtonState:ButtonEnabled];
        controller.wizardState = wizardState;
        controller.delegate = self;
        [controller initWizard];
        _wizardViewController = controller;
        [_titleLabel setStringValue:[controller title]];
    }
}

- (WizardViewController*)createViewController:(NSInteger)pageId {
    switch (pageId) {
        case WizardChooseSource:
            return [[ChooseSourceViewController alloc] initWithNibName:@"ChooseSourceViewController" bundle:nil];
        case WizardGogSource:
            return [[GogSourceViewController alloc] initWithNibName:@"GogSourceViewController" bundle:nil];
        case WizardSteamSource:
            return [[SteamSourceViewController alloc] initWithNibName:@"SteamSourceViewController" bundle:nil];
        case WizardCdromSource:
            return [[CdromSourceViewController alloc] initWithNibName:@"CdromSourceViewController" bundle:nil];
        case WizardTargetDir:
            return [[TargetDirViewController alloc] initWithNibName:@"TargetDirViewController" bundle:nil];
        case WizardExtracting:
            return [[ExtractingViewController alloc] initWithNibName:@"ExtractingViewController" bundle:nil];
        default:
            return nil;
    }
}

@end
