
const float TWO_PI      = 2.0 * 3.1415926;
const float GRID_LENGTH = 2.0;
const float PI          = 3.1415926;
const float R           = 1.0;
const float r           = 0.5;

/* inner shape variable */
const int SHAPE_GRID   = 0;
const int SHAPE_SPHERE = 1;
const int SHAPE_TORUS  = 2;

uniform bool uLocalViewer;
uniform bool uGpuGenShape;
uniform int  uTessellation;
uniform int  uInnerShapeType;

void gpuGrid(inout vec4 pos,out vec3 n)
{
  float i = float(pos.x) , j = float(pos.y);
  float start = -1.0;

  pos.x = start + i * GRID_LENGTH;
  pos.y = start + j * GRID_LENGTH;
  pos.z = 0.0;
  gl_Position   = gl_ModelViewProjectionMatrix * pos;


  n = vec3(0.0,0.0,1.0);
}

void gpuSphere(inout vec4 pos,out vec3 n)
{
  vec3 sphereNormal;
  float u = float(pos.x) , v = float(pos.y);

  pos.x = 1.0 * cos(TWO_PI * u) * sin(PI * v);
  pos.y = 1.0 * sin(TWO_PI * u) * sin(PI * v);
  pos.z = 1.0 * cos(PI * v);

  sphereNormal.x = cos(TWO_PI * u) * sin(PI * v);
  sphereNormal.y = sin(TWO_PI * u) * sin(PI * v);
  sphereNormal.z = cos(PI * v);

  gl_Position   = gl_ModelViewProjectionMatrix * pos;
  n = sphereNormal;
}

void gpuTorus(inout vec4 pos,out vec3 n)
{
  vec3 torusNormal;
  float u = float(pos.x) , v = float(pos.y);

  pos.x = (R + r * cos(TWO_PI * u)) * cos(TWO_PI * v);
  pos.y = (R + r * cos(TWO_PI * u)) * sin(TWO_PI * v);
  pos.z = r * sin(TWO_PI * u);

  torusNormal.x = cos( TWO_PI * u) * cos( TWO_PI * v);
  torusNormal.y = cos( TWO_PI * u) * sin( TWO_PI * v);
  torusNormal.z = sin( TWO_PI * u);

  gl_Position   = gl_ModelViewProjectionMatrix * pos;
  n = torusNormal;
}

void main(void)
{
  int tessellation = uTessellation;

  vec3 n = gl_Normal;
  vec4 pos = gl_Vertex;

  /* eyeVec in 4 space & 3 space */
  vec4 ecPosition;
  vec3 ecPosition3;

  if(uGpuGenShape == true) {
    if(uInnerShapeType == SHAPE_GRID) {
          gpuGrid(pos,n);
    }
    else if(uInnerShapeType == SHAPE_SPHERE) {
          gpuSphere(pos,n);
    }
    else if(uInnerShapeType == SHAPE_TORUS) {
          gpuTorus(pos,n);
    }
  }
  else {
        n = vec3(gl_Normal);
        gl_Position = ftransform();
  }

  vec3 normal, lightDir, eyeVec, reflection;
  vec4 diffuse, ambient, globalAmbient, specular = vec4(0.0);
  float NdotL,RdotE;

  /* transform the normal into eye space and normalize it */
  normal = normalize(gl_NormalMatrix * n);

  /* local view */
  if (uLocalViewer) {
    /* convert to eye space */
    ecPosition = gl_ModelViewMatrix * pos;
    ecPosition3 = (vec3(ecPosition)) / ecPosition.w;
    eyeVec = -normalize(ecPosition3);
  }
  else {
     /* get the eye vector, which is located at 0,0 */
     eyeVec = vec3(0.0, 0.0, 1.0);
  }

  /* normalize the light's direction. OpenGL store light in eye space and
    the position is direction */
  lightDir   = normalize(vec3(gl_LightSource[0].position));
  reflection = reflect(-lightDir,normal);

  /* compute the cos of the angle between the normal and lights direction.*/
  NdotL = max(dot(normal, lightDir), 0.0);

  /* Compute the diffuse, ambient and globalAmbient terms */
  diffuse = gl_FrontMaterial.diffuse    * gl_LightSource[0].diffuse;
  ambient = gl_FrontMaterial.ambient    * gl_LightSource[0].ambient;
  globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;

  /* compute the specular term if NdotL is  larger than zero */
  if (NdotL > 0.0) {

    RdotE = max(dot(reflection, eyeVec),0.0);
    specular = gl_FrontMaterial.specular * gl_LightSource[0].specular *
               pow(RdotE,gl_FrontMaterial.shininess);
  }

  gl_FrontColor = globalAmbient + NdotL * diffuse + ambient + specular;

}

