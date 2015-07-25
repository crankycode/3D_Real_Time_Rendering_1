//Per-vertex Blinn-Phong lighting

uniform bool uLocalViewer;
uniform bool uGpuGenShape;
uniform int  uTessellation;
uniform int  uInnerShapeType;

varying vec3 normal, lightDir, eyeVec;

void main()
{	
  
  	if(uGpuGenShape == true && uTessellation == 64)
{  
	normal = gl_NormalMatrix * gl_Normal;

	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);

	lightDir = vec3(gl_LightSource[0].position.xyz - vVertex);
	eyeVec = -vVertex;
}
	gl_Position = ftransform();		
}

