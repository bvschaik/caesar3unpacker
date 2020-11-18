//
//  SteamSourceViewController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "SteamSourceViewController.h"

@interface SteamSourceViewController ()

@end

@implementation SteamSourceViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)initWizard {
    [self.delegate setNextButtonState:ButtonHidden];
}

- (NSString *)title {
    return @"Steam";
}

@end
