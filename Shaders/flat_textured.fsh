//
//  flat textured - Fragment
//

varying highp vec2 fragment_texture_coordinates;
varying mediump vec4 fragment_color_multiply;
varying mediump vec4 fragment_color_lerp;

uniform sampler2D texture0;

void main()
{
    gl_FragColor = texture2D(texture0, fragment_texture_coordinates);
    gl_FragColor.r = gl_FragColor.r * (1.0 - fragment_color_lerp.a) + fragment_color_lerp.r * fragment_color_lerp.a * gl_FragColor.a;
    gl_FragColor.g = gl_FragColor.g * (1.0 - fragment_color_lerp.a) + fragment_color_lerp.g * fragment_color_lerp.a * gl_FragColor.a;
    gl_FragColor.b = gl_FragColor.b * (1.0 - fragment_color_lerp.a) + fragment_color_lerp.b * fragment_color_lerp.a * gl_FragColor.a;
    gl_FragColor.r *= fragment_color_multiply.r;
    gl_FragColor.g *= fragment_color_multiply.g;
    gl_FragColor.b *= fragment_color_multiply.b;
    gl_FragColor.a *= fragment_color_multiply.a;
    gl_FragColor.r = 1.0;
    gl_FragColor.a = 1.0;
}
