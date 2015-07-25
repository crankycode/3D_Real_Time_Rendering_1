//Per-vertex Blinn-Phong lighting
uniform bool uLocalViewer;
uniform bool uGpuGenShape;
uniform int  uTessellation;
uniform int  uInnerShapeType;

varying vec3 Besp;

void main(void)
{
    vec4 pos = gl_Vertex;
    vec3 torusNormal;

    float R = 1.0  , r = 0.5;
    float u = float(pos.x) , v = float(pos.y);
 
   
    pos.x = (R + r * cos(2.0 * 3.1415926 * u)) * cos(2.0 * 3.1415926 * v);
    pos.y = (R + r * cos(2.0 * 3.1415926 * u)) * sin(2.0 * 3.1415926 * v);
    pos.z = r * sin(2.0 * 3.1415926 * u);
    
    torusNormal.x = cos( 2.0 * 3.1415926 * u) * cos( 2.0 * 3.1415926 * v);
    torusNormal.y = cos( 2.0 * 3.1415926 * u) * sin( 2.0 * 3.1415926 * v);
    torusNormal.z = sin( 2.0 * 3.1415926 * u);
  
    
  
  float distance, displacement, dimensions;
  int   floorVal;

  
  vec2  dir;
  vec3  bumpNormal;
  float BumpSize       = 0.15;
  float SpecularFactor = 0.5;
  float BumpDensity    = 16.0;

 
  float d, f;
    
  vec2 px = vec2(gl_Vertex.xy);
  vec2 p  = px;

  vec3 tangent;
  vec3 EyeDir        = vec3(gl_ModelViewMatrix * gl_Vertex);
  vec3 SyslightDir   = vec3(gl_LightSource[0].position);

  vec3 n = normalize(gl_NormalMatrix * torusNormal);
  vec3 t = normalize(gl_NormalMatrix * tangent);
  vec3 b = cross(n, t);
  vec3 vector;

  tangent.x = -sin(u) * cos(v);
  tangent.y = -sin(u) * sin(v);
  tangent.z = cos(u);
  
  p *= 16;
  p = fract(p);
  dir = p - vec2(0.5,0.5);
  
  distance = sqrt(dir.x * dir.x + dir.y * dir.y);
  if(distance < 0.5) {
   displacement = sqrt(0.25f - distance * distance);
   bumpNormal = vec3(dir.x, dir.y, displacement) * 2;

   Besp = normalize((t * bumpNormal.x) + (b * bumpNormal.y ) + (n * bumpNormal.z )); 

  }
  else {
     Besp = torusNormal;
  }
  gl_Position =  gl_ModelViewProjectionMatrix * pos;
  

}

