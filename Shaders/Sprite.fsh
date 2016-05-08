//
//  Shader.fsh
//  NomNomCats
//
//  Created by Maxwell Robinson on 7/28/12.
//  Copyright (c) 2012 All The Loot. All rights reserved.
//

varying highp vec2 fragmentTexCoord;

uniform mediump vec4 colorModulation;
uniform mediump vec4 colorLerp;

uniform sampler2D texture0;

void main()
{
    gl_FragColor = texture2D(texture0, fragmentTexCoord);
    gl_FragColor.r = gl_FragColor.r * (1.0 - colorLerp.a) + colorLerp.r * colorLerp.a * gl_FragColor.a;
    gl_FragColor.g = gl_FragColor.g * (1.0 - colorLerp.a) + colorLerp.g * colorLerp.a * gl_FragColor.a;
    gl_FragColor.b = gl_FragColor.b * (1.0 - colorLerp.a) + colorLerp.b * colorLerp.a * gl_FragColor.a;
    gl_FragColor.r *= colorModulation.r;
    gl_FragColor.g *= colorModulation.g;
    gl_FragColor.b *= colorModulation.b;
    gl_FragColor.a *= colorModulation.a;
}
