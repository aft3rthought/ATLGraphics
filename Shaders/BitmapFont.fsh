//
//  Bitmap Font Render fragment shader
//

uniform mediump float u_antialiasingRadius;

varying lowp vec2 fv_texCoord;

uniform sampler2D s_texture;
uniform lowp vec4 s_fontColor;

void main()
{
    gl_FragColor = s_fontColor;
    gl_FragColor.a *= texture2D(s_texture, fv_texCoord).r;
    gl_FragColor.rgb *= gl_FragColor.a;
}
