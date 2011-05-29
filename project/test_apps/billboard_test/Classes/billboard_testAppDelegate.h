//
//  billboard_testAppDelegate.h
//  billboard_test
//
//  Created by Chris Wynn on 10/26/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class billboard_testViewController;

@interface billboard_testAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    billboard_testViewController *viewController;
}

@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) billboard_testViewController *viewController;

@end

