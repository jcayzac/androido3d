//
//  o3dappViewController.h
//  o3dapp
//
//  Created by Chris Wynn on 10/8/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "o3dManager.h"

@interface o3dappViewController : UIViewController
{
    EAGLContext *context;
    CADisplayLink *displayLink;
	O3DManager *o3dmgr;
}
- (void)startAnimation;
- (void)stopAnimation;
@end
