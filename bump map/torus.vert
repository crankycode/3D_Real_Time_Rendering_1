
//Per-vertex Blinn-Phong lighting

uniform bool LocalViewer;

uniform int blPerVertLight;
uniform int shaderBaseVertexGen;

void main(void)
{
  float distance, displacement, dimensions;
  int   floorVal;
  vec4 dir = gl_Vertex;
	vec3 normal, lightDir, viewVector, halfVector;
	vec4 diffuse, ambient, globalAmbient, specular = vec4(0.0);
	float NdotL,NdotHV;
	
	/* first transform the normal into eye space and normalize the result */
	normal = normalize(gl_NormalMatrix * gl_Normal);
	
	lightDir = normalize(vec3(gl_LightSource[0].position));
	
	NdotL = max(dot(normal, lightDir), 0.0);
	
	/* Compute the diffuse, ambient and globalAmbient terms */
	diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
	ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;
	
	/* compute the specular term if NdotL is  larger than zero */
	if (NdotL > 0.0) {

		NdotHV = max(dot(normal, normalize(gl_LightSource[0].halfVector.xyz)),0.0);
		specular = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(NdotHV,gl_FrontMaterial.shininess);
	}
	
	gl_FrontColor = globalAmbient + NdotL * diffuse + ambient + specular;

  distance = sqrt(dir.x * dir.x + dir.y * dir.y);
  floorVal = int(distance);
//  gl_FrontColor = vec4(floorVal,0,0,0);
  dimensions = distance - (float(floorVal)*2);
/*  
  if(distance < 0.5) {
    displacement = sqrt(0.25f - distance * distance);
    dir.z = displacement;
    gl_Position = gl_ModelViewProjectionMatrix * dir;
  }
  else {
    gl_Position = ftransform();
  }
*/  
  
  if(dimensions < 1.0 ) {
    displacement = sqrt(1.0f - distance * distance);
    dir.z = displacement;
    gl_Position = gl_ModelViewProjectionMatrix * dir;
  }
  else {
    gl_Position = ftransform();
  }

    
	
}

