
// default definitions
#include "./bvh-state-code.glsl"



// intersection current state
struct PrimitiveState {
     vec4 lastIntersection; // used from previosly only 
    ivec4 raydata; // with rayID
} primitiveState;

#define RAY_ID primitiveState.raydata[0]
#define MAX_TASK_COUNT primitiveState.raydata[1]



// alpha version of task pushing 
int pushTask(in int rayID, in int instanceID) {
    const int thid = atomicIncTaskCount(); imageStore(taskList, thid, uvec4(rayID+1, instanceID, 0u.xx)); // push a task to traversing 
    --MAX_TASK_COUNT; return thid;
};

// 
void doIntersection(in bool isvalid, in float dlen) {
    // TODO: need to correct naming 
    isvalid = isvalid && traverseState.defElementID > 0 && traverseState.defElementID <= traverseState.maxElements;
    IFANY (isvalid) {
        [[flatten]] if (isvalid) { pushTask(RAY_ID, traverseState.defElementID); }; // push a task to bottom level traversing 
    }; traverseState.defElementID=0;
};


// corrections of box intersection
const bvec4 bsgn = false.xxxx;
const 
float dirlen = 1.f, invlen = 1.f, bsize = 1.f;


// BVH traversing itself 
bool isLeaf(in ivec2 mem) { return mem.x==mem.y && mem.x >= 1; };
void resetEntry(in bool valid) { traverseState.idx = (valid ? BVH_ENTRY : -1), traverseState.stackPtr = 0, traverseState.pageID = 0, traverseState.defElementID = 0; };
void initTraversing( in bool valid, in int eht, in vec3 orig, in dirtype_t pdir, in int rayID ) {
    [[flatten]] if (eht.x >= 0) primitiveState.lastIntersection = hits[eht].uvt;
    RAY_ID = rayID+1, MAX_TASK_COUNT = 2; // for pushing a traversing tasks

    // relative origin and vector ( also, preparing mat3x4 support ) 
    // in task-based traversing will have universal transformation for BVH traversing and transforming in dimensions 
    const vec4 torig = -uniteBox(vec4(mult4(bvhBlock.transform, vec4(orig, 1.f)).xyz, 1.f)), torigTo = uniteBox(vec4(mult4(bvhBlock.transform, vec4(orig, 1.f) + vec4(dcts(pdir).xyz, 0.f)).xyz, 1.f)), tdir = torigTo+torig;
    const vec4 dirct = tdir*invlen, dirproj = 1.f / precIssue(dirct);
    //primitiveState.dir = primitiveState.orig = dirct;

    // test intersection with main box
    vec4 nfe = vec4(0.f.xx, INFINITY.xx);
    const   vec3 interm = fma(fpInner.xxxx, 2.f / (bvhBlock.sceneMax - bvhBlock.sceneMin), 1.f.xxxx).xyz;
    const   vec2 bside2 = vec2(-fpOne, fpOne);
    const mat3x2 bndsf2 = mat3x2( bside2*interm.x, bside2*interm.y, bside2*interm.z );

    // initial traversing state
    valid = valid && intersectCubeF32Single((torig*dirproj).xyz, dirproj.xyz, bsgn, bndsf2, nfe), resetEntry(valid);

    // traversing inputs
    traverseState.diffOffset = min(-nfe.x, 0.f);
    traverseState.directInv = fvec4_(dirproj), traverseState.minusOrig = fvec4_(vec4(fma(fvec4_(torig), traverseState.directInv, ftype_(traverseState.diffOffset).xxxx)));
    //primitiveState.orig = fma(primitiveState.orig, traverseState.diffOffset.xxxx, torig);
};

#include "./bvh-traverse-code.glsl"
