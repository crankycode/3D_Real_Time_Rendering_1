//Per-vertex Blinn-Phong lighting

uniform bool uLocalViewer;
uniform bool uGpuGenShape;
uniform int  uTessellation;
uniform int  uInnerShapeType;

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
  
    vec3 tangent;


    tangent.x = -sin(u) * cos(v);
    tangent.y = -sin(u) * sin(v);
    tangent.z = cos(u);
  
    vec3 Besp;
    vec3 n = normalize(gl_NormalMatrix * torusNormal);
    vec3 t = normalize(gl_NormalMatrix * tangent);
    vec3 b = cross(n, t);
    vec3 vector;
  
  float distance, displacement, dimensions;
  int   floorVal;

  
  vec2  dir;
  vec3  bumpNormal;
  float BumpSize       = 0.15;
  float SpecularFactor = 0.5;
  float BumpDensity    = 16.0;
  bool  bumpDect = false;
 
  float d, f;
  float bumpOffSet = 0.0;
  vec2 px = vec2(gl_Vertex.xy);
  vec2 p  = px;
  p *= 16;
  p = fract(p);
  dir = p - vec2(0.5,0.5);
  
  distance = sqrt(dir.x * dir.x + dir.y * dir.y);
  if(distance < 0.5 ) {
     displacement = sqrt(0.25f - distance * distance);
     bumpNormal = normalize(vec3(dir.x, dir.y, displacement) * 2);
     
     
     Besp = normalize((t * bumpNormal.x) + (b * bumpNormal.y ) + (n * bumpNormal.z ));
     pos += vec4(Besp,0)/20;
  }
  else {
     Besp = n;
  }
    
   vec3 normal, lightDir, viewVector, halfVector;
   vec4 diffuse, ambient, globalAmbient, specular = vec4(0.0);
   float NdotL,NdotHV;
    
    gl_Position =  gl_ModelViewProjectionMatrix * pos;
   /* now normalize the light's direction. Note that according to the
   OpenGL specification, the light is stored in eye space. Also since 
   we're talking about a directional light, the position field is actually 
   direction */
   lightDir = normalize(vec3(gl_LightSource[0].position));
   
   /* compute the cos of the angle between the normal and lights direction. 
   The light is directional so the direction is constant for every vertex.
   Since these two are normalized the cosine is the dot product. We also 
   need to clamp the result to the [0,1] range. */
   
   NdotL = max(dot(Besp, lightDir), 0.0);
   
   /* Compute the diffuse, ambient and globalAmbient terms */
   diffuse = gl_FrontMaterial.diffuse    * gl_LightSource[0].diffuse;
   ambient = gl_FrontMaterial.ambient    * gl_LightSource[0].ambient;
   globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;
   
   /* compute the specular term if NdotL is  larger than zero */
   if (NdotL > 0.0) {

      NdotHV = max(dot(Besp, normalize(gl_LightSource[0].halfVector.xyz)),0.0);
      specular = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(NdotHV,gl_FrontMaterial.shininess);
   }
   
   gl_FrontColor = globalAmbient + NdotL * diffuse + ambient + specular;

}

