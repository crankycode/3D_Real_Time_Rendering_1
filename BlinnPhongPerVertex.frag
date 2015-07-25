//shader.frag
uniform bool uLocalViewer;
uniform bool uGpuGenShape;
uniform int  uTessellation;
uniform int  uInnerShapeType;

void main (void)
{
	//gl_FragColor = vec4(0, 0, 1, 1);
        //gl_FragColor = vec4(0, 1, 1, 1);
        //gl_FragColor.rgb = vec3(distance);
        gl_FragColor = gl_Color;

}
