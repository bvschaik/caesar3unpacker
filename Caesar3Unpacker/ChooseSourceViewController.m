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

- (void)initWizard {
    [self.delegate setBackButtonState:ButtonHidden];
    ButtonState nextState = ButtonDisabled;
    if (self.wizardState.sourceId) {
        nextState = ButtonEnabled;
    }
    [self.delegate setNextButtonState:nextState];
}

- (NSString *)title {
    return @"Choose source";
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)selectSource:(id)sender {
    self.wizardState.sourceId = (Source) [sender tag];
    [self.delegate setNextButtonState:ButtonEnabled];
}

@end
