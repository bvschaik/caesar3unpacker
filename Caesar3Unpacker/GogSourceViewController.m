//
//  GogSourceViewController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "GogSourceViewController.h"

@interface GogSourceViewController ()

@end

@implementation GogSourceViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (NSString *)title {
    return @"Select GOG installer";
}

- (WizardPage)getNextPage {
    return WizardTargetDir;
}

- (WizardPage)getPrevPage {
    return WizardChooseSource;
}

- (void)browseForFile:(id)sender {
    
}

@end
