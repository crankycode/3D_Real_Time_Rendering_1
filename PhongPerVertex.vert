//Per-vertex Blinn-Phong lighting

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
  float gridStep = GRID_LENGTH/uTessellation;
  float start = -1.0;
  
  pos.x = start + gridStep * i; 
  pos.y = start + gridStep * j;
  pos.z = 0;
  gl_Position   = gl_ModelViewProjectionMatrix * pos; 
  
  n = vec3(0,-0,1);
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
  vec3 n = gl_Normal;
  vec4 pos = gl_Vertex;
  int tessellation = uTessellation;
  
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
  
  vec3 normal, lightDir, viewVector, halfVector, eyeVector, reflection;
  vec4 diffuse, ambient, globalAmbient, specular = vec4(0.0);
  float NdotL,RdotE;
 
  /* first transform the normal into eye space and normalize the result */
  normal    = normalize(gl_NormalMatrix * n);
  /* get the eye vector, which is located at 0,0 */
  eyeVector = normalize(-vec3(gl_ModelViewMatrix * pos));
  /* now normalize the light's direction. Note that according to the
  OpenGL specification, the light is stored in eye space. Also since 
  we're talking about a directional light, the position field is actually 
  direction */
  lightDir   = normalize(vec3(gl_LightSource[0].position));
  reflection = reflect(-lightDir,normal);
  /* compute the cos of the angle between the normal and lights direction. 
  The light is directional so the direction is constant for every vertex.
  Since these two are normalized the cosine is the dot product. We also 
  need to clamp the result to the [0,1] range. */
  
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
  
  gl_FrontColor = globalAmbient + NdotL * diffuse + ambient + specular;
  
   
}

