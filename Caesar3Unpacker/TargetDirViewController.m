//
//  TargetDirViewController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "TargetDirViewController.h"

@interface TargetDirViewController ()

@end

@implementation TargetDirViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)initWizard {
    if (self.wizardState.targetUrl == nil) {
        [self.delegate setNextButtonState:ButtonDisabled];
    } else {
        self.pathLabel.stringValue = self.wizardState.targetUrl.path;
    }
}

- (NSString *)title {
    return @"Select target";
}

- (void)browseForFolder:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:NO];
    [panel setCanChooseDirectories:YES];
    [panel setAllowsMultipleSelection:NO];

    NSInteger result = [panel runModal];

    if (result == NSFileHandlingPanelOKButton) {
        NSURL *url = [[panel URL] URLByAppendingPathComponent:@"Caesar 3"];
        self.wizardState.targetUrl = url;
        self.pathLabel.stringValue = url.path;
        [self.delegate setNextButtonState:ButtonEnabled];
    }
}

@end
