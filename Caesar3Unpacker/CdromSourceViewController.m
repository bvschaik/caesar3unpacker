//
//  CdromSourceViewController.m
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import "CdromSourceViewController.h"

@interface CdromSourceViewController ()

@end

@implementation CdromSourceViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)initWizard {
    if (self.wizardState.sourceUrl == nil) {
        [self.delegate setNextButtonState:ButtonDisabled];
    } else {
        self.pathLabel.stringValue = self.wizardState.sourceUrl.path;
    }
}

- (NSString *)title {
    return @"Select CD-ROM";
}

- (void)browseForFolder:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:NO];
    [panel setCanChooseDirectories:YES];
    [panel setAllowsMultipleSelection:NO];

    NSInteger result = [panel runModal];

    if (result == NSFileHandlingPanelOKButton) {
        NSURL *url = [panel URL];
        self.wizardState.sourceUrl = url;
        self.pathLabel.stringValue = url.path;
        [self.delegate setNextButtonState:ButtonEnabled];
    }
}

@end
