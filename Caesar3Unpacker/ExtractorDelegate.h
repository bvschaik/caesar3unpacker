//
//  ExtractorDelegate.h
//  Caesar3Unpacker
//
//  Created by bianca on 18-11-2020.
//  Copyright © 2020 Julius Team. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol ExtractorDelegate

- (void)onExtractorProgress:(NSString*)message;
- (void)onExtractorError:(NSString*)message;
- (void)onExtractorDone;

@end

NS_ASSUME_NONNULL_END
