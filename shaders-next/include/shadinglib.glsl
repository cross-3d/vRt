#ifndef _SHADINGLIB_H
#define _SHADINGLIB_H


//#define UNSHADE_BACKFACE


// gap in hit point
#define GAP (PZERO*2.f)

vec3 lightCenter(in int i) {
    return fma(normalize(lightUniform.lightNode[i].lightVector.xyz), lightUniform.lightNode[i].lightVector.www, lightUniform.lightNode[i].lightOffset.xyz);
}

vec3 sphereLightPoint(in int i) {
    return fma(randomDirectionInSphere(), vec3(lightUniform.lightNode[i].lightColor.w - 0.0001f), lightCenter(i));
}

vec3 getLightColor(in int lc) {
    return max(lightUniform.lightNode[lc].lightColor.xyz, vec3(0.f));
}


float intersectSphere(in vec3 origin, in vec3 ray, in vec3 sphereCenter, in float sphereRadius) {
    vec3 toSphere = origin - sphereCenter;
    float a = dot(ray, ray);
    float b = 2.0f * dot(toSphere, ray);
    float c = dot(toSphere, toSphere) - sphereRadius*sphereRadius;
    float discriminant = fma(b,b,-4.0f*a*c);
    float t = INFINITY;
    if (discriminant > 0.0f) {
        float da = 0.5f / a;
        float t1 = (-b - sqrt(discriminant)) * da;
        float t2 = (-b + sqrt(discriminant)) * da;
        float mn = min(t1, t2);
        float mx = max(t1, t2);
        t = mx >= 0.0f ? (mn >= 0.0f ? mn : mx) : t;
    }
    return t;
}

float modularize(in float f) {
    return 1.0f-sqrt(max(1.0f - f, 0.f));
}

float samplingWeight(in vec3 ldir, in vec3 ndir, in float radius, in float dist) {
    return modularize(max(dot(ldir, ndir), 0.f) * pow(radius / dist, 2.f)) * 2.f;
}


RayRework directLight(in int i, in RayRework ray, in vec3 color, in mat3 tbn) {
    RayActived(ray, RayType(ray) == 2 ? false_ : RayActived(ray));
    RayTargetLight(ray, i);
    RayDiffBounce(ray, min(1,max(RayDiffBounce(ray)-(RayType(ray)==3?0:1),0)));
        RayBounce(ray, min(1,max(    RayBounce(ray)-(RayType(ray)==3?0:0),0))); // incompatible with reflections and diffuses
    RayType(ray, 2);
    RayDL(ray, true_); // always illuminated by sunlight

    vec3 siden = faceforward(tbn[2], dcts(ray.cdirect.xy), tbn[2]);
    vec3 lpath = sphereLightPoint(i) - ray.origin.xyz;
    vec3 ldirect = normalize(lpath);
    float dist = length(lightCenter(i).xyz - ray.origin.xyz);
    float weight = samplingWeight(ldirect, siden, lightUniform.lightNode[i].lightColor.w, dist);

    ray.cdirect.xy = lcts(ldirect);
    ray.origin.xyz = fma(ldirect.xyz, vec3(GAP), ray.origin.xyz);
    WriteColor(ray.dcolor, f16_f32(ray.dcolor) * vec4(color,1.f) * vec4(weight.xxx,1.f));

    IF (lessF(dot(ldirect.xyz, siden), 0.f)) {
        RayActived(ray, false_); // wrong direction, so invalid
    }

    // any trying will fail when flag not enabled
#ifndef DIRECT_LIGHT_ENABLED
    RayActived(ray, false_);
#endif

    // ineffective
#ifndef ENABLE_PT_SUNLIGHT
    RayActived(ray, false_);
#endif

    // inactived can't be shaded
    // also, culling by normal
#ifdef UNSHADE_BACKFACE
    IF (not(RayActived(ray)) | bool_(dot(tbn[2], ldirect) < 0.f)) {
        WriteColor(ray.dcolor, 0.f.xxxx);
    }
#else
    IF (not(RayActived(ray))) { WriteColor(ray.dcolor, 0.f.xxxx); }
#endif 

    return ray;
}

RayRework diffuse(in RayRework ray, in vec3 color, in mat3 tbn) {
    WriteColor(ray.dcolor, f16_f32(ray.dcolor) * vec4(color,1.f));
    RayDL(ray, true_);

#ifdef USE_OPTIMIZED_PT
    const int diffuse_reflections = 1;
#else
    const int diffuse_reflections = 2;
#endif

    RayActived(ray, RayType(ray) == 2 ? false_ : RayActived(ray));
    RayDiffBounce(ray, min(diffuse_reflections, max(RayDiffBounce(ray)-(RayType(ray)==3?0:1),0)));

    vec3 siden = faceforward(tbn[2], dcts(ray.cdirect.xy), tbn[2]);
    vec3 sdr = randomCosineNormalOriented(rayStreams[RayDiffBounce(ray)].superseed[1], siden);
    sdr = faceforward(sdr, sdr, -siden);
    ray.cdirect.xy = lcts(sdr);
    ray.origin.xyz = fma(sdr, vec3(GAP), ray.origin.xyz);

    if (RayType(ray) != 2) RayType(ray, 1);

    // inactived can't be shaded
    // also, culling by normal
#ifdef UNSHADE_BACKFACE
    IF (not(RayActived(ray)) | bool_(dot(tbn[2], sdr) < 0.f)) {
        WriteColor(ray.dcolor, 0.f.xxxx);
    }
#else
    IF (not(RayActived(ray))) { WriteColor(ray.dcolor, 0.f.xxxx); }
#endif 

    return ray;
}


RayRework promised(in RayRework ray, in mat3 tbn) {
    ray.origin.xyz = fma(dcts(ray.cdirect.xy), vec3(GAP), ray.origin.xyz);
    IF (not(RayActived(ray))) WriteColor(ray.dcolor, 0.f.xxxx);
    return ray;
}


RayRework emissive(in RayRework ray, in vec3 color, in mat3 tbn) {
    WriteColor(ray.dcolor, max(f16_f32(ray.dcolor) * vec4(color,1.f), vec4(0.0f)));
    WriteColor(ray.dcolor, RayType(ray) == 2 ? 0.0f.xxxx : f16_f32(ray.dcolor));
    ray.origin.xyz = fma(dcts(ray.cdirect.xy), vec3(GAP), ray.origin.xyz);
    RayBounce(ray, 0);
    RayActived(ray, false_);

#ifdef UNSHADE_BACKFACE
    IF (bool_(dot(tbn[2], dcts(ray.cdirect.xy)) > 0.f)) { WriteColor(ray.dcolor, 0.f.xxxx); }
#endif 

    return ray;
}


RayRework reflection(in RayRework ray, in vec3 color, in mat3 tbn, in float gloss) {
    WriteColor(ray.dcolor, f16_f32(ray.dcolor) * vec4(color, 1.f));

#ifdef DISABLE_REFLECTIONS
    const int caustics_bounces = 0, reflection_bounces = 0; gloss = 0.f;
#else
    const int caustics_bounces = 0, reflection_bounces = 2;
#endif

    //if ( RayType(ray) == 3 ) RayDL(ray, true_); // allow direct speculars
    if ( RayType(ray) == 1 ) RayDL(ray, true_); // allow to caustics light
    RayBounce(ray, min(RayType(ray)==1?caustics_bounces:reflection_bounces, max(RayBounce(ray) - (RayType(ray)==3?0:1), 0)));
    if ( RayType(ray) != 2 ) RayType(ray, 0); // reflection ray transfer (primary)

    vec3 siden = faceforward(tbn[2], dcts(ray.cdirect.xy), tbn[2]);
    vec3 sdr = randomCosineNormalOriented(rayStreams[RayBounce(ray)].superseed[2], siden);
    sdr = faceforward(sdr, sdr, -siden);
    sdr = normalize(fmix(sdr, reflect(dcts(ray.cdirect.xy), siden), clamp(pow(1.f - random()*gloss, 2.f), 0.0f, 1.0f).xxx));
    sdr = faceforward(sdr, sdr, -siden);

    ray.cdirect.xy = lcts(sdr);
    ray.origin.xyz = fma(sdr, vec3(GAP), ray.origin.xyz);
    RayActived(ray, RayType(ray) == 2 ? false_ : RayActived(ray));

    // inactived can't be shaded
    // also, culling by normal
#ifdef UNSHADE_BACKFACE
    IF (not(RayActived(ray)) | bool_(dot(tbn[2], sdr) < 0.f)) {
        WriteColor(ray.dcolor, 0.f.xxxx);
    }
#else
    IF (not(RayActived(ray))) { WriteColor(ray.dcolor, 0.f.xxxx); }
#endif 

    return ray;
}

#endif
