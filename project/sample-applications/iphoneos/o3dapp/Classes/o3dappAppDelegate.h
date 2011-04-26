//
//  o3dappAppDelegate.h
//  o3dapp
//
//  Created by Chris Wynn on 10/8/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class o3dappViewController;

@interface o3dappAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    o3dappViewController *viewController;
}

@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) o3dappViewController *viewController;

@end

