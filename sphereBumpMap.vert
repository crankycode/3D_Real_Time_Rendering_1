varying vec3 LightDir;
varying vec3 EyeDir;
varying vec2 UV;

uniform vec3 LightPosition;
attribute vec3 Tangent;
varying vec3 normal,halfVector;
varying vec4 diffuse,ambient;

void main()
{
    vec4 pos = gl_Vertex;
    vec3 sphereNormal;
    vec3 SyslightDir;
    vec3 tangent;
    
    float R = 1.0  , r = 0.5;
    float u = float(pos.x) , v = float(pos.y);
    UV = vec2(pos.x,pos.y);
     
    pos.x = 1.0 * cos(2.0 * 3.1415926 * u) * sin(3.1415926 * v);
    pos.y = 1.0 * sin(2.0 * 3.1415926 * u) * sin(3.1415926 * v);
    pos.z = 1.0 * cos(3.1415926 * v);
        
    sphereNormal.x = cos(2.0 * 3.1415926 * u) * sin(3.1415926 * v);
    sphereNormal.y = sin(2.0 * 3.1415926 * u) * sin(3.1415926 * v);
    sphereNormal.z = cos(3.1415926 * v);
    
    gl_Position   = gl_ModelViewProjectionMatrix * pos;

    EyeDir        = vec3(gl_ModelViewMatrix * gl_Vertex);
    gl_Position   = gl_ModelViewProjectionMatrix * pos;
    SyslightDir   = vec3(gl_LightSource[0].position);
    
    tangent.x = 1.0;
    tangent.y = 0.0;
    tangent.z = pos.x/pos.z;
    
    vec3 n = normalize(gl_NormalMatrix * sphereNormal);
    vec3 t = normalize(gl_NormalMatrix * tangent);
    vec3 b = cross(n, t);
    vec3 vector;
    
    vector.x = dot(SyslightDir, t);
    vector.y = dot(SyslightDir, t);
    vector.z = dot(SyslightDir, n);
    LightDir = normalize(vector);

    vector.x = dot(EyeDir, t);
    vector.y = dot(EyeDir, t);
    vector.z = dot(EyeDir, n);
    EyeDir = normalize(vector);
    
    
      
   
      
 
}
