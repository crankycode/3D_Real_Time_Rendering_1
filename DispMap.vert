const float BASE_BUMP_SIZE = 0.5 / 50.0;
const float GRID_LENGTH    = 2.0;
const float TWO_PI         = 2.0 * 3.1415926;
const float PI             = 3.1415926;
const float R              = 1.0;
const float r              = 0.5;

/* inner shape variable */
const int SHAPE_GRID   = 0;
const int SHAPE_SPHERE = 1;
const int SHAPE_TORUS  = 2;

uniform bool uLocalViewer;
uniform bool uGpuGenShape;
uniform int  uTessellation;
uniform int  uInnerShapeType;
uniform int  uNumOfBumps;
uniform int  uCurrBumpSize;

void gpuGrid(inout vec4 pos, out vec3 tangent, out vec3 normal)
{
  float i = float(pos.x) , j = float(pos.y);
  float start = -1.0;

  pos.x = start + i * GRID_LENGTH;
  pos.y = start + j * GRID_LENGTH;
  pos.z = 0.0;
  gl_Position   = gl_ModelViewProjectionMatrix * pos;

  tangent.x = 1.0;
  tangent.y = 0.0;
  tangent.z = 0.0;

  normal = vec3(0,0,1);
}

void gpuSphere(inout vec4 pos, out vec3 tangent, out vec3 normal)
{
  vec3 sphereNormal;
  float u = float(pos.x) * TWO_PI , v = float(pos.y) * PI;

  pos.x = 1.0 * cos(u) * sin(v);
  pos.y = 1.0 * sin(u) * sin(v);
  pos.z = 1.0 * cos(v);

  sphereNormal.x = cos(u) * sin(v);
  sphereNormal.y = sin(u) * sin(v);
  sphereNormal.z = cos(v);

  tangent.x = -sin(u) * sin(v);
  tangent.y =  cos(u) * sin(v);
  tangent.z =  0.0;

  gl_Position   = gl_ModelViewProjectionMatrix * pos;
  normal = sphereNormal;
}

void gpuTorus(inout vec4 pos,out vec3 tangent, out vec3 normal)
{
  vec3 torusNormal;
  float u = float(pos.x) * TWO_PI , v = float(pos.y) * TWO_PI;

  pos.x = (R + r * cos(u)) * cos(v);
  pos.y = (R + r * cos(u)) * sin(v);
  pos.z = r * sin(u);

  torusNormal.x = cos(u) * cos(v);
  torusNormal.y = cos(u) * sin(v);
  torusNormal.z = sin(u);

  tangent.x = -sin(u) * cos(v);
  tangent.y = -sin(u) * sin(v);
  tangent.z = cos(u);

  gl_Position   = gl_ModelViewProjectionMatrix * pos;
  normal = torusNormal;
}

void main(void)
{
  /* displacement var */
  vec4  pos       = gl_Vertex;
  vec3  normal    = gl_Normal;
  vec3  tangent   = vec3(0.0,0.0,0.0);
  vec3  bumpNormal, Besp;
  vec2  dir;
  vec2  originLoc   = vec2(gl_Vertex.xy);
  vec2  perturbLoc  = originLoc;
  float bumpSize = BASE_BUMP_SIZE * float(uCurrBumpSize);
  float distance, displacement;

   /* eyeVec in 4 space & 3 space */
  vec4 ecPosition;
  vec3 ecPosition3;

  /* lighting var */
  vec3 lightDir,eyeVector, reflection;
  vec4 diffuse, ambient, globalAmbient, specular = vec4(0.0);
  float NdotL,RdotE;

  if(uGpuGenShape == true) {
    if(uInnerShapeType == SHAPE_GRID) {
          gpuGrid(pos,tangent,normal);
    }
    else if(uInnerShapeType == SHAPE_SPHERE) {
          gpuSphere(pos,tangent,normal);
    }
    else if(uInnerShapeType == SHAPE_TORUS) {
          gpuTorus(pos,tangent,normal);
    }
  }
  else {
        normal = vec3(gl_Normal);
        gl_Position = ftransform();
  }

  /* create transformation matrix */
  vec3 n = normalize(gl_NormalMatrix * normal);
  vec3 t = normalize(gl_NormalMatrix * tangent);
  vec3 b = cross(t,n);

  perturbLoc *= float(uNumOfBumps);
  perturbLoc  = fract(perturbLoc);

  dir = perturbLoc - vec2(float(bumpSize),float(bumpSize));

  distance = float(sqrt(dir.x * dir.x + dir.y * dir.y));

  if (distance < bumpSize ) {
      displacement = sqrt(bumpSize * bumpSize - distance * distance);
      bumpNormal = normalize(vec3(dir.x, dir.y, displacement) * 2.0);

    if (uInnerShapeType == SHAPE_GRID)
        Besp = bumpNormal;
    else
        Besp = normalize((t * bumpNormal.x) + (b * bumpNormal.y ) + (n * bumpNormal.z ));
    /* displace the vertex */
    pos.xyz += normal * (2.0*displacement/float(uNumOfBumps));
  }
  else {
     Besp = n;
  }

   gl_Position =  gl_ModelViewProjectionMatrix * pos;
     /* local view */
  if (uLocalViewer) {
    /* convert to eye space */
    ecPosition = gl_ModelViewMatrix * pos;
    ecPosition3 = (vec3(ecPosition)) / ecPosition.w;
    eyeVector = -normalize(ecPosition3);
  }
  else {
    /* get the eye vector, which is located at 0,0 */
    eyeVector = vec3(0.0, 0.0, 1.0);
  }

   /* normalize the light's direction. OpenGL store light in eye space and
    the position is direction */
   lightDir = normalize(vec3(gl_LightSource[0].position));

   /* compute reflection */
   reflection = reflect(-lightDir,Besp);

   /* compute the cos of the angle between the normal and lights direction. */
   NdotL = max(dot(Besp, lightDir), 0.0);

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
  //gl_FrontColor = vec4(t.xyz, 1.0);

}

