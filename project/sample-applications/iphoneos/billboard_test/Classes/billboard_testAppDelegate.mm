//
//  billboard_testAppDelegate.m
//  billboard_test
//
//  Created by Chris Wynn on 10/26/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "billboard_testAppDelegate.h"
#import "billboard_testViewController.h"

@implementation billboard_testAppDelegate

@synthesize window;
@synthesize viewController;

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	// create and initialize the window (normally IB does this for you)
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	
	// Start to create the initial controller and view (this is typically where your code starts)
	viewController = [[billboard_testViewController alloc] init];
	
	// Configure and show the window (e.g. add you view to the window
	[window addSubview:[viewController view]];
	
	// Now show the window
	[window makeKeyAndVisible];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    [viewController stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    [viewController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    [viewController stopAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Handle any background procedures not related to animation here.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Handle any foreground procedures not related to animation here.
}

- (void)dealloc
{
    [viewController release];
    [window release];
    
    [super dealloc];
}

@end