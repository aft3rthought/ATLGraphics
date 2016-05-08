//
//  Screen Edge effect shader - Vertex
//

uniform highp vec4 u_screenBounds;
uniform vec4 u_color;
uniform vec2 u_scale;
uniform vec2 u_offset;

attribute vec2 v_vertPosition;
attribute lowp vec4 v_vertColor;

varying lowp vec4 fragment_color;

void main()
{
    // Output final vert position in screen coordinates:
    gl_Position = vec4(-1.0 + (v_vertPosition.x * u_scale.x + u_offset.x - u_screenBounds.x) / u_screenBounds.z * 2.0,
                       -1.0 + (v_vertPosition.y * u_scale.y + u_offset.y - u_screenBounds.y) / u_screenBounds.w * 2.0,
                       0.0,
                       1.0);

    fragment_color = vec4(v_vertColor.r * u_color.r,
                                v_vertColor.g * u_color.g,
                                v_vertColor.b * u_color.b,
                                v_vertColor.a * u_color.a);
}
