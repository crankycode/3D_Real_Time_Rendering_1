//Per-vertex Blinn-Phong lighting

uniform bool LocalViewer;

uniform int blPerVertLight;
uniform int shaderBaseVertexGen;

varying vec3 normal, lightDir, eyeVec;

void main()
{	
  vec3 vVertex, lightDir;
  vec3 N, L, E, R;
  vec4 final_color;
  float specular, NdotL;
  
	normal = gl_NormalMatrix * gl_Normal;

	vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);

	lightDir = vec3(gl_LightSource[0].position.xyz - vVertex);
	eyeVec = -vVertex;
  
  final_color = (gl_FrontLightModelProduct.sceneColor * 
                 gl_FrontMaterial.ambient) + (gl_LightSource[0].ambient * 
                 gl_FrontMaterial.ambient);
							
	N = normalize(normal);
	L = normalize(lightDir);
  E = normalize(eyeVec);
  R = reflect(-L, N);
  
	NdotL = dot(N,L);
	
	if(NdotL > 0.0)
	{
		final_color += gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * NdotL;	
	
    specular = pow( max(dot(R, E), 0.0), gl_FrontMaterial.shininess );
		final_color += gl_LightSource[0].specular * gl_FrontMaterial.specular * 
					         specular;	
	}
  gl_FrontColor = final_color;

	gl_Position = ftransform();		
}

