varying vec3 LightDir;
varying vec3 EyeDir;
varying vec2 UV;

uniform vec3 LightPosition;
attribute vec3 Tangent;

void main()
{
    vec4 pos = gl_Vertex;
    vec3 torusNormal;
    vec3 SyslightDir;
    vec3 tangent;
    
    float R = 1.0f  , r = 0.5f;
    float u = float(pos.x) , v = float(pos.y);
    UV = vec2(pos.x,pos.y);
   
    pos.x = (R + r * cos(2.0 * 3.1415926 * u)) * cos(2.0 * 3.1415926 * v);
    pos.y = (R + r * cos(2.0 * 3.1415926 * u)) * sin(2.0 * 3.1415926 * v);
    pos.z = r * sin(2.0 * 3.1415926 * u);
    
    torusNormal.x = cos( 2.0 * 3.1415926 * u) * cos( 2.0 * 3.1415926 * v);
    torusNormal.y = cos( 2.0 * 3.1415926 * u) * sin( 2.0 * 3.1415926 * v);
    torusNormal.z = sin( 2.0 * 3.1415926 * u);
    
    gl_Position   = gl_ModelViewProjectionMatrix * pos;

    EyeDir        = vec3(gl_ModelViewMatrix * gl_Vertex);
    gl_Position   = gl_ModelViewProjectionMatrix * pos;
    SyslightDir   = vec3(gl_LightSource[0].position);
    
    tangent.x = cos(u) * (-sin(v));
    tangent.y = sin(u) * (-sin(v));
    tangent.z = cos(v);
    
    vec3 n = normalize(gl_NormalMatrix * torusNormal);
    vec3 t = normalize(gl_NormalMatrix * tangent);
    vec3 b = cross(n, t);
    vec3 vector;
    
    vector.x = dot(SyslightDir, t);
    vector.y = dot(SyslightDir, b);
    vector.z = dot(SyslightDir, n);
    LightDir = normalize(vector);

    vector.x = dot(EyeDir, t);
    vector.y = dot(EyeDir, b);
    vector.z = dot(EyeDir, n);
    EyeDir = normalize(vector);
 
}
