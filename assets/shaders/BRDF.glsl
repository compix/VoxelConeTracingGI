#ifndef BRDF_GLSL
#define BRDF_GLSL

#define BLINN_PHONG_MODE_IDX 0
#define COOK_TORRANCE_MODE_IDX 1

float fresnel(vec3 viewVector, vec3 halfway)
{
    return pow(1.0 - dot(viewVector, halfway), 5);
}

vec3 fresnelSchlick(vec3 F0, vec3 l, vec3 n)
{
    float p = pow(1.0 - max(0.0, dot(l, n)), 5);
    return F0 + (1.0 - F0) * p;
}

vec3 fresnelSchlickMicrofacet(vec3 F0, vec3 l, vec3 h)
{
    float p = pow(1.0 - dot(l, h), 5);
    return F0 + (1.0 - F0) * p;
}

vec3 blinnPhongBRDF(vec3 Id, vec3 kd, vec3 Is, vec3 ks, vec3 normal, vec3 lightVec, vec3 halfway, float shininess)
{
    float diffuseFactor = max(dot(normal, lightVec), 0.0);
    vec3 diffuse = Id * kd * diffuseFactor;
    
    float specFactor = pow(max(dot(normal, halfway), 0.0), shininess);
    vec3 specular = vec3(0.0);
    
    if (shininess > 0.0 && diffuseFactor > 0.0)
        specular = Is * ks * specFactor;
    
    return diffuse + specular;
}

float beckmannNDF(vec3 n, vec3 h, float roughness)
{
    float rSq = roughness * roughness;
    float nh = max(0.0, dot(n,h));
    float nhSq = nh * nh;
    float nhPow4 = nhSq * nhSq;
    
    float c0 = 1.0 / (rSq * nhPow4);
    float c1 = exp(-(1.0 - nhSq) / (nhSq * rSq));
    
    return c0 * c1;
}

vec3 cookTorranceBRDF(vec3 l, vec3 n, vec3 v, vec3 h, float roughness, vec3 F0)
{
    vec3 F = fresnelSchlick(F0, l, h);
    
    float nh = max(0.0, dot(n,h));
    float vh = max(0.0, dot(v,h));
    float nv = max(0.0, dot(n,v));
    float nl = max(0.0, dot(n,l));
    float G0 = (2.0 * nh * nv) / vh;
    float G1 = (2.0 * nh * nl) / vh;
    
    float G = min(1.0, max(0.0, min(G0, G1)));
    
    float D = beckmannNDF(n, h, roughness);
    
    vec3 numerator = F * G * D;
    float denominator = 4.0 * nl * nv;
    
    return max(vec3(0.0), numerator / denominator);
}

#endif