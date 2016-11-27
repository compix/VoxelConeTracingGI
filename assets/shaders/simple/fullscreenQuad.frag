#version 100
precision mediump float;

varying vec2 v_texCoords;

uniform sampler2D u_textureDiffuse;
uniform float u_isGrayscale;

void main() 
{
    gl_FragColor = texture2D(u_textureDiffuse, v_texCoords);

    if (u_isGrayscale > 0.0)
        gl_FragColor = vec4(gl_FragColor.rrr, 1.0);
}
