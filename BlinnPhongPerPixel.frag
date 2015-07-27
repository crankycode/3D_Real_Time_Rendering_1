uniform bool uLocalViewer;
uniform bool uGpuGenShape;
uniform int  uTessellation;
uniform int  uInnerShapeType;

varying vec3 normal,lightDir,halfVector;

void main()
{
  vec3 n,ldir;
  vec4 diffuse, ambient, globalAmbient, specular = vec4(0.0);
  float NdotL,NdotHV;
  vec4 color = ambient;

  /* normalize the normal again */
  n = normalize(normal);

  /* cal the cos between the light and the normal */
  NdotL = max(dot(n, normalize(lightDir)), 0.0);

  /* Compute the diffuse, ambient and globalAmbient terms */
  diffuse       = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
  ambient       = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
  globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;

  /* compute the specular term if NdotL is  larger than zero */
  if (NdotL > 0.0) {

    NdotHV = max(dot(n, normalize(halfVector)),0.0);
    specular = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(NdotHV,gl_FrontMaterial.shininess);
  }
  /* add up all the terms */
  gl_FragColor = globalAmbient + NdotL * diffuse + ambient + specular;
}
