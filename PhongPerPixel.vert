
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

varying vec3 eyeVec,normalVec,lightDirVec;

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
  vec3 n = gl_Normal;
  vec4 pos = gl_Vertex;

  /* eyeVec in 4 space & 3 space */
  vec4 ecPosition;
  vec3 ecPosition3;

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


  /* normalize the normal */
  normalVec   = normalize(n);
  /* position of the light is the direction vector */
  lightDirVec = vec3(gl_LightSource[0].position.xyz);

}

