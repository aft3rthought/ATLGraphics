//
//  Distance Field Font Render (Simple Version) fragment shader
//

uniform mediump float u_edges[4];
uniform lowp vec4 u_colors[4];
uniform mediump float u_antialiasingRadius;

varying mediump float fv_index;
varying lowp vec2 fv_texCoord;

uniform sampler2D s_distanceField;

void main()
{
    //        1-     2-     3-    4-     1+     0+     -1+     -2+
    // 0      1      2      3     4      1       0       0       0
    // 1      0      1      2     3      2       1       0       0
    // 2      0      0      1     2      3       2       1       0
    // 3      0      0      0     1      4       3       2       1
    
    mediump vec4 factors = vec4(max(1.0 - fv_index, 0.0) * max( 1.0 + fv_index, 0.0),
                                max(2.0 - fv_index, 0.0) * max( 0.0 + fv_index, 0.0),
                                max(3.0 - fv_index, 0.0) * max(-1.0 + fv_index, 0.0),
                                max(4.0 - fv_index, 0.0) * max(-2.0 + fv_index, 0.0));
    
    mediump float edge = u_edges[0] * factors[0] + u_edges[1] * factors[1] + u_edges[2] * factors[2] + u_edges[3] * factors[3];
    mediump vec4 color = u_colors[0] * factors[0] + u_colors[1] * factors[1] + u_colors[2] * factors[2] + u_colors[3] * factors[3];

    mediump float dfVal = texture2D(s_distanceField, fv_texCoord).r;
    
    gl_FragColor.a = color.a * clamp((dfVal - edge) / u_antialiasingRadius, 0.0, 1.0);
    gl_FragColor.rgb = color.rgb * gl_FragColor.a;

    /*
    mediump float l_dfVal = texture2D(s_distanceField, fv_texCoord).r;

    int l_index = int(fv_index);
    
    mediump float edge = u_edges[l_index];
    lowp vec4 color = u_colors[l_index];
    
    gl_FragColor.a = color.a * clamp((l_dfVal - edge) / u_antialiasingRadius, 0.0, 1.0);
    gl_FragColor.rgb = color.rgb * gl_FragColor.a;
     */
}
