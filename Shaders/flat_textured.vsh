//
//  flat textured - Vertex
//

uniform highp vec4 u_screenBounds;

attribute highp vec2 v_position;
attribute highp vec2 v_texture_coordinates;
attribute mediump vec4 v_color_multiply;
attribute mediump vec4 v_color_lerp;

varying highp vec2 fragment_texture_coordinates;
varying mediump vec4 fragment_color_multiply;
varying mediump vec4 fragment_color_lerp;

void main()
{
    // Output final vert position in screen coordinates:
    gl_Position = vec4(-1.0 + (v_position.x - u_screenBounds.x) / u_screenBounds.z * 2.0,
                       -1.0 + (v_position.y - u_screenBounds.y) / u_screenBounds.w * 2.0,
                       0.0,
                       1.0);

    fragment_texture_coordinates = v_texture_coordinates;
    fragment_color_multiply = v_color_multiply;
    fragment_color_lerp = v_color_lerp;
}
