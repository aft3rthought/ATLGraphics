//
//  Distance Field Font Rendering (Simple Version) vertex shader
//

uniform highp vec4 u_screenBounds;
uniform vec2 u_translation;

attribute vec2 v_position;
attribute lowp vec2 v_texCoord;
attribute mediump float v_index;

varying lowp vec2 fv_texCoord;
varying mediump float fv_index;
void main()
{
    // Translate into screen coordinates:
    // Output final vert position in screen coordinates:
    gl_Position = vec4(-1.0 + (v_position.x + u_translation.x - u_screenBounds.x) / u_screenBounds.z * 2.0,
                       -1.0 + (v_position.y + u_translation.y - u_screenBounds.y) / u_screenBounds.w * 2.0,
                       0.0,
                       1.0);

    // Per-vertex pass through:
    fv_texCoord = v_texCoord;
    fv_index = v_index;
}
