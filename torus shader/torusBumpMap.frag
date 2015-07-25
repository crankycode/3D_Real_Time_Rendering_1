varying vec3 LightDir;
varying vec3 EyeDir;
varying vec2 UV;

void main()
{

vec3 SurfaceColor    = vec3(0.7, 0.6, 0.18); 
float BumpSize       = 0.15;
float SpecularFactor = 0.5;
float BumpDensity    = 16.0;
 
    vec3 litColor;
    vec2 c = BumpDensity * UV.xy;
    vec2 p = fract(c) - vec2(0.5);

    float d, f;
    d = p.x * p.x + p.y * p.y;
    f = 1.0 / sqrt(d + 1.0);

    if (d >= BumpSize)
        { p = vec2(0.0); f = 1.0; }

    vec3 normDelta = vec3(p.x, p.y, 1.0) * f;
    litColor = SurfaceColor * max(dot(normDelta, LightDir), 0.0);
    vec3 reflectDir = reflect(LightDir, normDelta);

    float spec = max(dot(EyeDir, reflectDir), 0.0);
    spec = pow(spec, 6.0);
    spec *= SpecularFactor;
    litColor = min(litColor + spec, vec3(1.0));

    gl_FragColor = vec4(litColor, 1.0);

}