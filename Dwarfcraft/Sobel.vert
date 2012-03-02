// A basic Sobel edge detection shader.  Detects edges in
// primaryUnit texture, and using a threshold either returns
// a fragment as black (silhouette or crease) or as the color
// stored in colorUnit texture.

// This vertex program does nothing.

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = ftransform();
}
