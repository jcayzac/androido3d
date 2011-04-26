//
//  Shader.fsh
//  o3dapp
//
//  Created by Chris Wynn on 10/8/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
