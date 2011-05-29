//
//  camspacequad_testViewController.m
//  camspacequad_test
//
//  Created by Chris Wynn on 11/19/10.
//  Copyright 2010 Tonchidot Corporation. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "camspacequad_testViewController.h"
#import "EAGLView.h"

@interface camspacequad_testViewController ()
@property (nonatomic, retain) EAGLContext *context;
@end

@implementation camspacequad_testViewController

@synthesize animating, context;

- (void)loadView
{
	EAGLView *myView = [[EAGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	self.view = myView;
	[myView release];
	
    EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    if (!aContext)
    {
        aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    }
    
    if (!aContext)
        NSLog(@"Failed to create ES context");
    else if (![EAGLContext setCurrentContext:aContext])
        NSLog(@"Failed to set ES context current");
    
	self.context = aContext;
	[aContext release];
	
    [(EAGLView *)self.view setContext:context];
    [(EAGLView *)self.view setFramebuffer];
    
	// Create the O3D Manager
	o3dmgr = new O3DManager();
	o3dmgr->Initialize( [((EAGLView*)self.view) framebufferWidth], [((EAGLView*)self.view) framebufferHeight] );
	// Note: There is a bug
	//  1. You must call ResizeViewport() so that the camera and projection matrices are determine
	//  2. You must call Render() one time before calling ResizeViewport to force loading of some lazy loaded components
	//  3. Calling ResizeViewport() every frame will result in weird behaviour because the ResizeViewport uses the
	//     bounding box of the model at the current time-step when determining camera and projection matrices
	o3dmgr->Render();
	o3dmgr->ResizeViewport( [((EAGLView*)self.view) framebufferWidth], [((EAGLView*)self.view) framebufferHeight] );
	
    animating = FALSE;
    displayLinkSupported = FALSE;
    animationFrameInterval = 1;
    displayLink = nil;
    animationTimer = nil;
    
    // Use of CADisplayLink requires iOS version 3.1 or greater.
	// The NSTimer object is used as fallback when it isn't available.
    NSString *reqSysVer = @"3.1";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
        displayLinkSupported = TRUE;
}

- (void)dealloc
{
	delete o3dmgr;
	o3dmgr = NULL;
	
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
    
    [context release];
    
    [super dealloc];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];
    
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];
    
    [super viewWillDisappear:animated];
}

- (void)viewDidUnload
{
	[super viewDidUnload];

    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	self.context = nil;	
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;
        
        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating)
    {
        if (displayLinkSupported)
        {
            /*
			 CADisplayLink is API new in iOS 3.1. Compiling against earlier versions will result in a warning, but can be dismissed if the system version runtime check for CADisplayLink exists in -awakeFromNib. The runtime check ensures this code will not be called in system versions earlier than 3.1.
            */
            displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawFrame)];
            [displayLink setFrameInterval:animationFrameInterval];
            
            // The run loop will retain the display link on add.
            [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        }
        else
            animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawFrame) userInfo:nil repeats:TRUE];
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        if (displayLinkSupported)
        {
            [displayLink invalidate];
            displayLink = nil;
        }
        else
        {
            [animationTimer invalidate];
            animationTimer = nil;
        }
        
        animating = FALSE;
    }
}

- (void)drawFrame
{
    if ( [(EAGLView *)self.view setFramebuffer] )
	{
		if (!o3dmgr->OnContextRestored()) {
			LOGI("Failed to restore resources");
			o3dmgr->CheckError();
		}
	}
	
	o3dmgr->Render();
	o3dmgr->CheckError();
	
    [(EAGLView *)self.view presentFramebuffer];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

@end
