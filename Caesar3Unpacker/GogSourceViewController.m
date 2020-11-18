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

- (void)initWizard {
    [self.delegate setNextButtonState:ButtonDisabled];
}

- (NSString *)title {
    return @"Select GOG installer";
}

- (void)browseForFile:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
    [panel setAllowedFileTypes:[NSArray arrayWithObject:@"exe"]];

    NSInteger result = [panel runModal];
    
    if (result == NSFileHandlingPanelOKButton) {
        NSURL *url = [panel URL];
        self.wizardState.sourceUrl = url;
        self.pathLabel.stringValue = url.path;
        [self.delegate setNextButtonState:ButtonEnabled];
    }
}

@end
