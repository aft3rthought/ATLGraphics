//
//  Shader.vsh
//  NomNomCats
//
//  Created by Maxwell Robinson on 7/28/12.
//  Copyright (c) 2012 All The Loot. All rights reserved.
//

uniform highp vec2 spriteCorners[4];
uniform highp vec2 spriteTexCoords[4];
uniform highp vec4 u_screenBounds;

attribute mediump vec4 vertComponents;

varying highp vec2 fragmentTexCoord;

void main()
{
    // Output final vert position in screen coordinates:
    highp vec2 l_pos = spriteCorners[0] * vertComponents.x +
                       spriteCorners[1] * vertComponents.y +
                       spriteCorners[2] * vertComponents.z +
                       spriteCorners[3] * vertComponents.w;

    gl_Position = vec4(-1.0 + (l_pos.x - u_screenBounds.x) / u_screenBounds.z * 2.0,
                       -1.0 + (l_pos.y - u_screenBounds.y) / u_screenBounds.w * 2.0,
                       0.0,
                       1.0);

    // Output tex coord:
    fragmentTexCoord = spriteTexCoords[0] * vertComponents.x +
                       spriteTexCoords[1] * vertComponents.y +
                       spriteTexCoords[2] * vertComponents.z +
                       spriteTexCoords[3] * vertComponents.w;
}
