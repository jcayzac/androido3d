//
//  o3dappViewController.m
//  o3dapp
//
//  Created by Chris Wynn on 10/8/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "o3dappViewController.h"
#import "EAGLView.h"

@interface o3dappViewController ()
@property (nonatomic, retain) EAGLContext *context;
@end

@implementation o3dappViewController
@synthesize context;

- (void)awakeFromNib {
    EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (!aContext)
        NSLog(@"Failed to create ES context");
    else if (![EAGLContext setCurrentContext:aContext])
        NSLog(@"Failed to set ES context current");

	self.context = aContext;
	[aContext release];

    [(EAGLView *)self.view setContext:context];
    [(EAGLView *)self.view setFramebuffer];

	// Create the O3D Manager
	o3dmgr = new O3DManager([((EAGLView*)self.view) framebufferWidth], [((EAGLView*)self.view) framebufferHeight] );
	// Note: There is a bug
	//  1. You must call ResizeViewport() so that the camera and projection matrices are determine
	//  2. You must call Render() one time before calling ResizeViewport to force loading of some lazy loaded components
	//  3. Calling ResizeViewport() every frame will result in weird behaviour because the ResizeViewport uses the
	//     bounding box of the model at the current time-step when determining camera and projection matrices
	o3dmgr->Render();
	o3dmgr->ResizeViewport( [((EAGLView*)self.view) framebufferWidth], [((EAGLView*)self.view) framebufferHeight] );
    displayLink = nil;
}

- (void)dealloc {
	delete o3dmgr;
	o3dmgr = 0;

    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];

    [context release];
    [super dealloc];
}

- (void)viewWillAppear:(BOOL)animated {
    [self startAnimation];
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated {
    [self stopAnimation];
    [super viewWillDisappear:animated];
}

- (void)viewDidLoad {
	[super viewDidLoad];
}

- (void)viewDidUnload {
	[super viewDidUnload];
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	self.context = nil;
}

- (void)startAnimation {
    if (!displayLink) {
		displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawFrame)];
		[displayLink setFrameInterval:1];
		[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
}

- (void)stopAnimation {
    if (displayLink) {
		[displayLink invalidate];
		displayLink = nil;
    }
}

- (void)drawFrame {
    if ( [(EAGLView *)self.view setFramebuffer] ) {
		if (!o3dmgr->OnContextRestored()) {
			LOGI("Failed to restore resources");
			o3dmgr->CheckError();
		}
	}

	o3dmgr->Render();
	o3dmgr->CheckError();

    [(EAGLView *)self.view presentFramebuffer];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

@end
