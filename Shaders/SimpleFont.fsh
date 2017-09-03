//
//  Distance Field Font Render (Simple Version) fragment shader
//

varying lowp vec2 fv_texCoord;
varying lowp vec4 fv_color;
varying mediump float fv_edge;
varying mediump float fv_radius;

uniform sampler2D s_distanceField;

void main()
{
    mediump float dfVal = texture2D(s_distanceField, fv_texCoord).r;
    
    gl_FragColor.a = fv_color.a * clamp((dfVal - fv_edge) / fv_radius, 0.0, 1.0);
    gl_FragColor.rgb = fv_color.rgb * gl_FragColor.a;
}
