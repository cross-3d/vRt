
// 
int traverseBVH2( in bool validTop ) {
    [[flatten]] if (validTop) {
        INSTANCE_ID = 0, LAST_INSTANCE = 0, currentState = uint(bvhBlockTop.primitiveCount <= 1), initTraversing(validTop, -1, ORIGINAL_ORIGIN, ORIGINAL_DIRECTION);
    };
    [[flatten]] if (validTop) [[dependency_infinite]] for (uint hi=0;hi<maxIterations;hi++) {  // two loop based BVH traversing
        [[flatten]] if (validIdx(traverseState.idx)) {
        { [[dependency_infinite]] for (;hi<maxIterations;hi++) {
            //IFANY (!validIdx(traverseState.idx) || traverseState.defElementID > 0) { break; }; // 

            bool _continue = false;
            #define bvhNode bvhNodes[traverseState.idx]
            const ivec2 cnode = validIdx(traverseState.idx) ? bvhNode.meta.xy : (0).xx;
            [[flatten]] if ( isLeaf(cnode.xy)) { traverseState.defElementID = VTX_PTR + cnode.x; } else  // if leaf, defer for intersection 
            [[flatten]] if (isnLeaf(cnode.xy)) { // if not leaf, intersect with nodes

#ifdef EXPERIMENTAL_UNORM16_BVH
                #define bbox2x fvec4_[3](\
                    fvec4_(validIdx(traverseState.idx)?unpackSnorm4x16(bvhNode.cbox[0]):0.f.xxxx),\
                    fvec4_(validIdx(traverseState.idx)?unpackSnorm4x16(bvhNode.cbox[1]):0.f.xxxx),\
                    fvec4_(validIdx(traverseState.idx)?unpackSnorm4x16(bvhNode.cbox[2]):0.f.xxxx)\
                )
#else
                #define bbox2x (validIdx(traverseState.idx)?bvhNode.cbox:fvec4_[3](0.f.xxxx,0.f.xxxx,0.f.xxxx)) // use same memory
#endif

                vec4 nfe = vec4(0.f.xx, INFINITY.xx);
                pbvec2_ childIntersect = bool(cnode.x&1) ? intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, bsgn, bbox2x, nfe) : false2_;
                

                // found simular technique in http://www.sci.utah.edu/~wald/Publications/2018/nexthit-pgv18.pdf
                // but we came up in past years, so sorts of patents may failure 
                // also, they uses hit queue, but it can very overload stacks, so saving only indices...
                childIntersect &= binarize(lessThanEqual(nfe.xy, fma(primitiveState.lastIntersection.z,fpOne,fpInner).xx)); // it increase FPS by filtering nodes by first triangle intersection

                //
                int primary = -1, secondary = -1;
                pbool_ fmask = pl_x(childIntersect)|(pl_y(childIntersect)<<true_);
                [[flatten]] if (fmask > 0 && fmask != -1) {
                    [[flatten]] if (fmask == 3) { fmask &= true_<<pbool_(nfe.x>nfe.y); secondary = cnode.x^int(fmask>>1u); }; // if both has intersection
                    primary = traverseState.entryIDBase + (cnode.x^int(fmask&1u));

                    // pre-intersection that triangle, because any in-stack op can't check box intersection doubly or reuse
                    // also, can reduce useless stack storing, and make more subgroup friendly triangle intersections
                    [[flatten]] if (currentState == BVH_STATE_BOTTOM && secondary > 0) {
                        const ivec2 snode = bvhNodes[traverseState.entryIDBase+secondary].meta.xy;
                        [[flatten]] if (isLeaf(snode)) { traverseState.defElementID = VTX_PTR + snode.x; secondary = -1; }; 
                    };
                _continue = true; };

                traverseState.idx = primary;
                [[flatten]] if (secondary > 0) storeStack(traverseState.entryIDBase+secondary);
            };
            
            // if all threads had intersection, or does not given any results, break for processing
            [[flatten]] if (!_continue) 
                { traverseState.idx = -1; loadStack(traverseState.idx); }; // load from stack
            IFANY (!validIdx(traverseState.idx) || traverseState.defElementID > 0 || traverseState.idx == bvhBlockTop.entryID) { break; }; // 
        }}};
        
        // every-step solving 
        [[flatten]] IFANY (traverseState.defElementID > 0) { doIntersection( validTop ); };
        [[flatten]] if (validIdxTop(traverseState.idxTop) && !validIdx(traverseState.idx) && traverseState.idx != bvhBlockTop.entryID) {
            switchStateTo(BVH_STATE_TOP, INSTANCE_ID, validTop);
        };
        [[flatten]] if (!validIdx(traverseState.idx) || traverseState.idx == bvhBlockTop.entryID) { break; };
    };
    return floatBitsToInt(primitiveState.lastIntersection.w);
};
