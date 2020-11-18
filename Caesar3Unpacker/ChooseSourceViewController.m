//
//  ChooseSourceViewController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "ChooseSourceViewController.h"

@interface ChooseSourceViewController ()

@end

@implementation ChooseSourceViewController

- (NSString *)title {
    return @"Choose source";
}

- (WizardPage)getNextPage {
    switch (self.wizardState.sourceId) {
        case 1: return WizardGogSource;
        case 2: return WizardSteamSource;
        case 3: return WizardCdromSource;
        default: return WizardNone;
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)selectSource:(id)sender {
    self.wizardState.sourceId = (Source) [sender tag];
}

@end
