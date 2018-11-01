#ifndef _HITLIB_H
#define _HITLIB_H

#ifndef SETS_DESC_SET_ID
#define SETS_DESC_SET_ID 1
#endif

const uint MAX_IMAGES = 256, MAX_SAMPLERS = 16;

// textrue/sampler set
// alternate of https://mynameismjp.wordpress.com/2016/03/25/bindless-texturing-for-deferred-rendering-and-decals/
layout ( binding = 0, set = SETS_DESC_SET_ID ) uniform texture2D images[MAX_IMAGES];
layout ( binding = 1, set = SETS_DESC_SET_ID ) uniform sampler samplers[MAX_SAMPLERS];

// material set (in main descriptor set)
layout ( binding = 2, set = SETS_DESC_SET_ID, std430 ) readonly restrict buffer VT_MATERIAL_BUFFER { VtAppMaterial submats[]; };
layout ( binding = 3, set = SETS_DESC_SET_ID, std430 ) readonly restrict buffer VT_COMBINED { u16vec2 vtexures[]; }; // TODO: replace by native combinations
layout ( binding = 4, set = SETS_DESC_SET_ID, std430 ) readonly restrict buffer VT_MATERIAL_INFO { uint materialCount, materialOffset; };


int matID = -1;
#define material submats[matID]

// validate texture object
bool validateTexture(in uint tbinding) { return tbinding > 0u && tbinding != -1u && int(tbinding) > 0; };

// also fixes AMD Vega texturing issues with nonuniformEXT
//#define sampler2Dv(m) sampler2D(images[vtexures[m].x-1], samplers[vtexures[m].y-1])
//#define fetchTexture(tbinding, tcoord) textureLod(sampler2Dv(nonuniformEXT(tbinding-1)), tcoord, 0)


#ifdef ENABLE_NON_UNIFORM_SAMPLER
#define sampler2Dv(m) sampler2D(images[nonuniformEXT(vtexures[m].x-1)],samplers[nonuniformEXT(vtexures[m].y-1)]) // TODO: replace by native combinations
#define fetchTexture(tbinding, tcoord) textureLod(sampler2Dv(nonuniformEXT(tbinding-1)),tcoord,0)
#else
#define sampler2Dv(m) sampler2D(images[vtexures[m].x-1],samplers[vtexures[m].y-1]) // TODO: replace by native combinations
#define fetchTexture(tbinding, tcoord) textureLod(sampler2Dv(tbinding-1),tcoord,0)
#endif


// new texture fetcher with corner sampling 
vec4 fetchTexNG(in uint tbinding, in vec2 ntxc) {
    
    /*
#ifdef AMD_PLATFORM
    ivec2 szi = (1).xx; bool found = false;
    // planned Turing hardware version support 
    for (uint i=0;i<Wave_Size_RT;i++) { // critical section, due AMD hardware has issues
        i = max(i, Wave_Size_RT-(1u+subgroupBallotFindMSB(subgroupBallot(!found))) );
        [[flatten]] if (!found && subgroupBroadcast(tbinding, i) == tbinding) {
            szi = textureSize(images[vtexures[tbinding-1].x-1],0), found = true;
        };
        [[flatten]] if (subgroupAll(found)) break;
    };
#else
    const ivec2 szi = textureSize(images[nonuniformEXT(vtexures[tbinding-1].x-1)],0);
#endif
    
    // shifting by half of texel may used in Mineways models
    //const vec2 sz = vec2(szi), tx = ntxc-0.5f/sz;
    const vec2 sz = vec2(szi), tx = corneredCoordinates(ntxc, sz);
    const vec2 is = 1.f/sz, tc = fma(tx,sz,-0.5f.xx), tm = (floor(tc+SFN)+0.5f)*is;
    const vec4 il = vec4(fract(tc),1.f-fract(tc)), cf = vec4(il.z*il.y,il.x*il.y,il.x*il.w,il.z*il.w);
    return mult4(mat4(textureGather(sampler2Dv(tbinding-1),tm,0),textureGather(sampler2Dv(tbinding-1),tm,1),textureGather(sampler2Dv(tbinding-1),tm,2),textureGather(sampler2Dv(tbinding-1),tm,3)),cf);
    */

    return fetchTexture(tbinding,ntxc); // or just use TPU
};

#define fetchTex(tbinding, tcoord) fetchTexNG(tbinding,tcoord)



vec4 fetchDiffuse(in vec2 texcoord) {
    const uint tbinding = material.diffuseTexture;
    const vec4 rslt = validateTexture(tbinding) ? fetchTex(tbinding, texcoord) : material.diffuse;
    return rslt;
};

vec4 fetchSpecular(in vec2 texcoord) {
    const uint tbinding = material.specularTexture;
    const vec4 rslt = validateTexture(tbinding) ? fetchTex(tbinding, texcoord) : material.specular;
    return rslt;
};

vec4 fetchEmission(in vec2 texcoord) {
    const uint tbinding = material.emissiveTexture;
    const vec4 rslt = validateTexture(tbinding) ? fetchTex(tbinding, texcoord) : material.emissive;
    return rslt;
};









#ifdef ENABLE_POM
const float parallaxScale = 0.02f;
const float minLayers = 10, maxLayers = 20;
const int refLayers = 10;
vec2 parallaxMapping(in vec3 V, in vec2 T, out float parallaxHeight) {
    const uint tbinding = material.bumpTexture;

    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), V)));
    float layerHeight = 1.0f / numLayers;
    vec2 dtex = parallaxScale * V.xy / (V.z) * layerHeight;
    vec3 chv = vec3(-dtex, layerHeight);
    
    // start pos
    vec3 chd_a = vec3(T, 0.f), chd_b = chd_a;

    // parallax sample tracing 
    [[unroll]] for (int l=0;l<256;l++) {
        float heightFromTexture = 1.f-fetchTex(tbinding, chd_b.xy).z;
        if ( heightFromTexture <= chd_b.z ) break;
        chd_a = chd_b, chd_b += chv;
    }
    
    // refinement
    [[unroll]] for (int l=0;l<refLayers;l++) {
        vec3 chd = mix(chd_a, chd_b, 0.5f);
        float heightFromTexture = 1.f-fetchTex(tbinding, chd.xy).z;
        if ( heightFromTexture <= chd.z ) { chd_b = chd; } else { chd_a = chd; }
    }

    // do occlusion
    float nextH = (1.f-fetchTex(tbinding, chd_b.xy).z) - chd_b.z;
    float prevH = (1.f-fetchTex(tbinding, chd_a.xy).z) - chd_a.z;

    float dvh = (nextH - prevH);
    float weight = nextH / (dvh);
    
    parallaxHeight = chd_b.z+mix(nextH, prevH, weight);
    return mix(chd_b.xy, chd_a.xy, weight);
}
#endif


// generated normal mapping
vec3 getUVH(in vec2 texcoord) { return vec3(texcoord, fetchTex(material.bumpTexture, texcoord).x); }
vec4 getNormalMapping(in vec2 texcoordi) {
    const uint tbinding = material.bumpTexture;
    const vec4 tc = validateTexture(tbinding) ? fetchTex(tbinding, texcoordi) : vec4(0.5f, 0.5f, 1.f, 1.f);

    vec4 normal = vec4(0.f,0.f,1.f,tc.w);
    if ( abs(tc.x-tc.y)<1e-4f && abs(tc.x-tc.z)<1e-4f ) {
        //vec2 txs = 1.f/textureSize(sampler2D(images[tbinding], samplers[0]), 0);
        vec2 txs = 1.f/textureSize(images[tbinding], 0);
        vec4 tx4 = vec4(-txs, txs)*0.5f;
        vec4 txu = vec4(-.5f.xx,.5f.xx);

        const float hsize = 2.f;
        vec3 t00 = vec3(txu.xy, getUVH(texcoordi + tx4.xy).z) * vec3(1.f, 1.f, hsize);
        vec3 t01 = vec3(txu.xw, getUVH(texcoordi + tx4.xw).z) * vec3(1.f, 1.f, hsize);
        vec3 t10 = vec3(txu.zy, getUVH(texcoordi + tx4.zy).z) * vec3(1.f, 1.f, hsize);
        vec3 bump = normalize(cross( t01 - t00, t10 - t00 ))*vec3(1.f,-1.f,1.f);
        normal.xyz = faceforward(bump, -normal.xyz, bump);
    } else {
        normal.xyz = normalize(mix(vec3(0.0f, 0.0f, 1.0f), fma(tc.xyz, vec3(2.0f), vec3(-1.0f)), vec3(1.0f)));//, normal.y *= -1.f;
        //normal.x *= -1.f;
    }

    return point4(normal, tc.w);
}


#endif
