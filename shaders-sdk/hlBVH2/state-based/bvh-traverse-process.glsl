
// 
int traverseBVH2( in bool validTop ) {
    {   currentState = uint(bvhBlockTop.primitiveCount <= 1), LAST_INSTANCE = -1, INSTANCE_ID = currentState == BVH_STATE_BOTTOM ? 0 : -1;
        traverseState.idx = (traverseState.entryIDBase = BVH_ENTRY), lstack[traverseState.stackPtr = 0] = -1, traverseState.pageID =  0, 
        traverseState.saved = false, traverseState.idxTop = -1, traverseState.stackPtrTop = 0, traverseState.pageIDTop = 0, traverseState.defElementID = 0;
    };
    initTraversing(validTop, -1, ORIGINAL_ORIGIN, ORIGINAL_DIRECTION);
    
    // two loop based BVH traversing
    [[flatten]] if (validIdx(traverseState.idx)) [[dependency_infinite]] for (uint hi=0;hi<maxIterations;hi++) {
        [[flatten]] if (validIdx(traverseState.idx)) {
        { [[dependency_infinite]] for (;hi<maxIterations;hi++) {

            //int primary = -1; 
            const uint lastDataID=uint(1+((currentState==BVH_STATE_TOP)?-1:bvhInstance.bvhDataID));
            #define bvhNode bvhNodes[traverseState.idx]
            const ivec2 cnode = validIdx(traverseState.idx) ? bvhNode.meta.xy : (0).xx;
            [[flatten]] if ( isLeaf(cnode.xy)) { traverseState.defElementID = VTX_PTR + cnode.x; traverseState.idx = -1; } else  // if leaf, defer for intersection 
            [[flatten]] if (isnLeaf(cnode.xy)) { // if not leaf, intersect with nodes
                #define bbox2x bvhNode.cbox//traverseState.idx

                vec4 nfe = vec4(0.f.xx, INFINITY.xx);
                pbvec2_ childIntersect = intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, bsgn, bbox2x, nfe);
                //pbvec2_ childIntersect = mix(false2_, intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, bsgn, bbox2x, nfe), bool(cnode.x&1));
                
                // found simular technique in http://www.sci.utah.edu/~wald/Publications/2018/nexthit-pgv18.pdf
                // but we came up in past years, so sorts of patents may failure 
                // also, they uses hit queue, but it can very overload stacks, so saving only indices...
                //childIntersect &= binarize(lessThanEqual(nfe.xy, fma(primitiveState.lastIntersection.z,fpOne,fpInner).xx)); // it increase FPS by filtering nodes by first triangle intersection
                childIntersect &= binarize(lessThanEqual(nfe.xy, primitiveState.lastIntersection.zz));

                //
                pbool_ fmask = pl_x(childIntersect)|(pl_y(childIntersect)<<true_); traverseState.idx = -1;
                [[flatten]] if (fmask > 0 && fmask != -1) {
                    int secondary = -1;
                    [[flatten]] if (fmask == 3) { fmask &= true_<<pbool_(nfe.x>nfe.y); secondary = cnode.x^int(fmask>>1u); }; // if both has intersection
                    traverseState.idx = traverseState.entryIDBase + (cnode.x^int(fmask&1u));

                    // pre-intersection that triangle, because any in-stack op can't check box intersection doubly or reuse
                    // also, can reduce useless stack storing, and make more subgroup friendly triangle intersections
                    [[flatten]] if (currentState == BVH_STATE_BOTTOM && secondary > 0) {
                        const ivec2 snode = bvhNodes[traverseState.entryIDBase+secondary].meta.xy;
                        [[flatten]] if (isLeaf(snode)) { traverseState.defElementID = VTX_PTR + snode.x; secondary = -1; }; 
                    };
                    [[flatten]] if (secondary > 0) storeStack(traverseState.entryIDBase+secondary);
                };
            };
            
            // if all threads had intersection, or does not given any results, break for processing
            //traverseState.idx = primary;
            [[flatten]] if (!validIdxEntry(traverseState.idx)) { loadStack(traverseState.idx); }; // load from stack
            IFANY (!validIdxEntry(traverseState.idx) || traverseState.defElementID > 0) { break; };
        }}};
        
        // every-step solving 
        bool stateSwitched = false; doIntersection( stateSwitched );
        [[flatten]] if (!validIdxIncluse(traverseState.idx) || (!stateSwitched && traverseState.idx <= traverseState.entryIDBase)) { break; };
    };
    
    return floatBitsToInt(primitiveState.lastIntersection.w);
};
