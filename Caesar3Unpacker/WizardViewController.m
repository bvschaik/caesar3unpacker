//
//  WizardViewController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "WizardViewController.h"

@interface WizardViewController ()

@end

@implementation WizardViewController

- (NSString *)title {
    return @"";
}

- (WizardPage)getNextPage {
    return WizardNone;
}

- (WizardPage)getPrevPage {
    return WizardNone;
}

@end
