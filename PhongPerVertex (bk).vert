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

void gpuGrid(out vec3 n)
{
  vec4  pos = gl_Vertex;
  float i = float(pos.x) , j = float(pos.y);
  float gridStep = GRID_LENGTH/uTessellation;
  float start = -1.0;
  
  pos.x = start + gridStep * i; 
  pos.y = start + gridStep * j;
  pos.z = 0;
  gl_Position   = gl_ModelViewProjectionMatrix * pos; 
  
  n = vec3(0,-0,1);
}

void gpuSphere(out vec3 n)
{
  vec4 pos = gl_Vertex;
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

void gpuTorus(out vec3 n)
{
  vec4 pos = gl_Vertex;
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
  vec3 n =gl_Normal;
  vec4 pos;
  int tessellation = uTessellation;
  if(uGpuGenShape == true) {
    if(uInnerShapeType == SHAPE_GRID) {
          gpuGrid(n);
    }
    else if(uInnerShapeType == SHAPE_SPHERE) {
          gpuSphere(n);
    }
    else if(uInnerShapeType == SHAPE_TORUS) {
          gpuTorus(n);
    }
  }      
  else {
        n = vec3(gl_Normal);
        gl_Position = ftransform();
  }


  vec3 vNormalEye; // normal vector in eye space
  vec3 vLightEye; // light direction vector
  vec3 vViewEye; // view (eye) vector
  
  vec4 uLightPos = gl_LightSource[0].position; // light position

   // transform input vertex position from
   // object space to clip space

   // transform input vertex position from
   // object space to eye space
   vec3 Peye =  vec3(gl_Position);

   // compute the light vector in eye space
   vLightEye = vec3(uLightPos);

   // transform the normal from object space
   // to eye space    
   vNormalEye = gl_NormalMatrix * n;

   // compute the eye vector in eye space;
   // this is easy because the eye is at the
   // origin, by definition
   vViewEye= -normalize(Peye);
   
   // compute various vectors and normalize them
   vec3 N = normalize(vNormalEye);
   vec3 L = normalize(vLightEye);
   vec3 R = reflect(L, N);
   vec3 V = normalize(vViewEye);
   vec3 H = normalize(L+V);

   // do diffuse+specular Phong / Blinn-Phong lighting
   float diffuse = max(dot(N, L), 0.0);
   float specPhong = pow(max(dot(R, V), 0.0), gl_FrontMaterial.shiness);
   float specBlinnPhong = pow(max(dot(N, H), 0.0), gl_FrontMaterial.shiness);
   if (dot(N, L) < 0.0)
   {
       specPhong = 0.0;
       specBlinnPhong = 0.0;
   }
   gl_FrontColor = gl_LightSource[0].ambient * gl_FrontMaterial.ambient + 
                  (diffuse * gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse) +
                  (specPhong * gl_LightSource[0].specular * gl_FrontMaterial.specular);
}

