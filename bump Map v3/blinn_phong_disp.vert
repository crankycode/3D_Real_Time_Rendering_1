//Per-vertex Blinn-Phong lighting
uniform int ShaderGenShape;
uniform bool LocalViewer;
uniform int blPerVertLight;


void main(void)
{
  
  float distance, displacement, dimensions;
  int   floorVal;
  vec4 pos = gl_Vertex;
  vec2 dir;
  vec3 bumpNormal;
  /* lighting */
	vec3 normal, lightDir, viewVector, halfVector;
	vec4 diffuse, ambient, globalAmbient, specular = vec4(0.0);
	float NdotL,NdotHV;
	
 
  vec2 p = vec2(pos);
  p *= 8;
  p = fract(p);
  dir = p - vec2(0.5,0.5);
  
  distance = sqrt(dir.x * dir.x + dir.y * dir.y);
  if(distance < 0.5) {
    displacement = sqrt(0.25f - distance * distance);
 //   pos.z = displacement/16;
    bumpNormal = vec3(dir.x, dir.y, displacement) * 2;
//    pos =vec4(pos + (vec4(bumpNormal,0)));
//    gl_Normal = bumpNormal;
//    pos =vec4(pos + (vec4(gl_Normal,0) * (displacement/8)));
//    gl_Position = gl_ModelViewProjectionMatrix * pos;
      gl_Position = ftransform();
  }
  else {
     gl_Position = ftransform();
   }
   
//   if(ShaderGenShape == 0 )
//   {
	/* first transform the normal into eye space and normalize the result */
	normal = normalize(gl_NormalMatrix * bumpNormal);
	
	/* now normalize the light's direction. Note that according to the
	OpenGL specification, the light is stored in eye space. Also since 
	we're talking about a directional light, the position field is actually 
	direction */
	lightDir = normalize(vec3(gl_LightSource[0].position));
	
	/* compute the cos of the angle between the normal and lights direction. 
	The light is directional so the direction is constant for every vertex.
	Since these two are normalized the cosine is the dot product. We also 
	need to clamp the result to the [0,1] range. */
	
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

//}
    
  
//  floorVal = int(distance);
//  gl_FrontColor = vec4(floorVal,0,0,0);
//  dimensions = distance - (float(floorVal)*2);
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
/*  
  if(dimensions < 0.5 ) {
    displacement = sqrt(0.25f - distance * distance);
    pos.z = displacement;
    gl_Position = gl_ModelViewProjectionMatrix * pos;
  }
  else {
    gl_Position = ftransform();
  }
*/	
	
}

