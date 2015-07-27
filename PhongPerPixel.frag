uniform bool uLocalViewer;
uniform bool uGpuGenShape;
uniform int  uTessellation;
uniform int  uInnerShapeType;

varying vec3 eyeVec,normalVec,lightDirVec;

void main (void)
{
  vec3  normal, lightDir, reflection, eyeVector;
  vec4  diffuse, ambient, globalAmbient, specular = vec4(0.0);
  float NdotL, RdotE;

  normal     = normalize(normalVec);
  lightDir   = normalize(lightDirVec);
  eyeVector  = normalize(eyeVec);
  reflection = reflect(-lightDir,normal);

  /* compute the cos of the angle between the normal and lights direction.*/
  NdotL = max(dot(normal, lightDir), 0.0);

  /* Compute the diffuse, ambient and globalAmbient terms */
  diffuse = gl_FrontMaterial.diffuse    * gl_LightSource[0].diffuse;
  ambient = gl_FrontMaterial.ambient    * gl_LightSource[0].ambient;
  globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;

  /* compute the specular term if NdotL is  larger than zero */
  if (NdotL > 0.0) {

    RdotE = max(dot(reflection, eyeVector),0.0);
    specular = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(RdotE,gl_FrontMaterial.shininess);
  }

  gl_FragColor = globalAmbient + NdotL * diffuse + ambient + specular;
}
