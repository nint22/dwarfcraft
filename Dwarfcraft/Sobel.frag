// A basic Sobel edge detection shader.  Detects edges in
// primaryUnit texture, and using a threshold either returns
// a fragment as black (silhouette or crease) or as the color
// stored in colorUnit texture

uniform sampler2D ColorBuffer;
uniform sampler2D DepthBuffer;
uniform float threshold;

void main(void)
{
    const float offset = 1.0 / 800.0;
    vec2 texCoord = gl_TexCoord[0].xy;
    
    // the texture offset values
    //        tl  t    tr
    //         l    c    r
    //        bl    b    br
    
    // Only apply the Sobel edge to the depth
    float c  = texture2D(DepthBuffer, texCoord).x;
    float bl = texture2D(DepthBuffer, texCoord + vec2(-offset, -offset)).x;
    float l  = texture2D(DepthBuffer, texCoord + vec2(-offset,     0.0)).x;
    float tl = texture2D(DepthBuffer, texCoord + vec2(-offset,  offset)).x;
    float t  = texture2D(DepthBuffer, texCoord + vec2(    0.0,  offset)).x;
    float tr = texture2D(DepthBuffer, texCoord + vec2( offset,  offset)).x;
    float r  = texture2D(DepthBuffer, texCoord + vec2( offset,     0.0)).x;
    float br = texture2D(DepthBuffer, texCoord + vec2( offset,  offset)).x;
    float b  = texture2D(DepthBuffer, texCoord + vec2(    0.0, -offset)).x;
    
    // the Sobel edge detection kernels
    //  x = -1  0  1    y = -1 -2 -1
    //      -2  0  2         0  0  0
    //      -1  0  1         1  2  1
    
    float x = -(tl + 2.0 * l + bl) + (tr + 2.0 * r + br);
    float y = -(tl + 2.0 * t + tr) + (bl + 2.0 * b + br);
    
    float pixel = sqrt(x * x + y * y);
    
    if (pixel >= threshold)
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    else
        gl_FragColor = texture2D(ColorBuffer, texCoord);
}
