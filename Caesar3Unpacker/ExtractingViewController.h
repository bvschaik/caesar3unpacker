//
//  ExtractingViewController.h
//  Caesar3Unpacker
//
//  Created by bianca on 17-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "WizardViewController.h"

NS_ASSUME_NONNULL_BEGIN

@interface ExtractingViewController : WizardViewController {
    BOOL shouldCancel;
}

@property (strong) IBOutlet NSTextView *progressView;

- (IBAction)onCancel:(id)sender;

@end

NS_ASSUME_NONNULL_END
