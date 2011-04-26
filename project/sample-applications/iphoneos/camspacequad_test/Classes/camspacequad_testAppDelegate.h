//
//  camspacequad_testAppDelegate.h
//  camspacequad_test
//
//  Created by Chris Wynn on 11/19/10.
//  Copyright 2010 Tonchidot Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@class camspacequad_testViewController;

@interface camspacequad_testAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    camspacequad_testViewController *viewController;
}

@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) camspacequad_testViewController *viewController;

@end

