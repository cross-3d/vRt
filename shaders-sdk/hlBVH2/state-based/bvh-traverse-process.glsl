
// 
int traverseBVH2( in bool validTop ) {
    {   
        currentState = uint(topLevelEntry == -1), LAST_INSTANCE = -1, INSTANCE_ID = currentState == BVH_STATE_BOTTOM ? 0 : -1;
        stackState = BvhSubState(traverseState.entryIDBase = BVH_ENTRY, mint_t(0), mint_t(0)), resrvState = BvhSubState(-1, mint_t(0), mint_t(0));
        initTraversing(true, -1, ORIGINAL_ORIGIN, ORIGINAL_DIRECTION);
    };
    
    // two loop based BVH traversing
    bool stateSwitched = false;
    [[flatten]] if (validIdx(stackState.idx)) [[dependency_infinite]] for (uint hi=0;hi<maxIterations;hi++) 
    {
        //[[flatten]] if (validIdx(stackState.idx)) //[[dependency_infinite]] for (;hi<maxIterations;hi++) 
        {
#ifdef ENABLE_MULTI_BVH // when enabled, can cause big performance drops
            const uint lastDataID=uint(1+((currentState==BVH_STATE_TOP)?-1:bvhInstance.bvhDataID));
#endif

            #define bvhNode bvhNodes[stackState.idx]
            const ivec2 cnode = validIdx(stackState.idx) ? bvhNode.meta.xy : (0).xx;
            [[flatten]] if ( isLeaf(cnode.xy)) { traverseState.defElementID = VTX_PTR + cnode.x, stackState.idx = -1; } else  // if leaf, defer for intersection 
            [[flatten]] if (isnLeaf(cnode.xy)) { // if not leaf, intersect with nodes
                #define bbox2x bvhNode.cbox

                vec4 nfe = vec4(0.f.xx, INFINITY.xx);
                pbvec2_ childIntersect = intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, bsgn, bbox2x, nfe);
                //pbvec2_ childIntersect = mix(false2_, intersectCubeDual(traverseState.minusOrig.xyz, traverseState.directInv.xyz, bsgn, bbox2x, nfe), bool(cnode.x&1));

                // found simular technique in http://www.sci.utah.edu/~wald/Publications/2018/nexthit-pgv18.pdf
                // but we came up in past years, so sorts of patents may failure 
                // also, they uses hit queue, but it can very overload stacks, so saving only indices...
                //childIntersect &= binarize(lessThanEqual(nfe.xy, fma(primitiveState.lastIntersection.z,fpOne,fpInner).xx)); // it increase FPS by filtering nodes by first triangle intersection
                childIntersect &= binarize(lessThanEqual(nfe.xy, primitiveState.lastIntersection.zz));

                //
                pbool_ fmask = pl_x(childIntersect)|(pl_y(childIntersect)<<true_);
                [[flatten]] if (fmask > 0 && fmask != -1) {
                    int secondary = -1;
                    [[flatten]] if (fmask == 3) { fmask &= true_<<pbool_(nfe.x>nfe.y); secondary = cnode.x^int(fmask>>1u); }; // if both has intersection
                    stackState.idx = traverseState.entryIDBase + (cnode.x^int(fmask&1u));

                    // pre-intersection that triangle, because any in-stack op can't check box intersection doubly or reuse
                    // also, can reduce useless stack storing, and make more subgroup friendly triangle intersections
                    [[flatten]] if (currentState == BVH_STATE_BOTTOM && secondary > 0) {
                        const ivec2 snode = bvhNodes[traverseState.entryIDBase+secondary].meta.xy;
                        [[flatten]] if (isLeaf(snode)) { traverseState.defElementID = VTX_PTR + snode.x; secondary = -1; }; 
                    };
                    [[flatten]] if (secondary > 0) storeStack(traverseState.entryIDBase+secondary);
                } else { stackState.idx = -1; };
            };
            
            // if all threads had intersection, or does not given any results, break for processing
            //stackState.idx = primary;
            [[flatten]] if (!validIdxEntry(stackState.idx)) { loadStack(stackState.idx); }; // load from stack
            //IFANY (!validIdxEntry(stackState.idx) || traverseState.defElementID > 0) { break; };
        };
        
        // every-step solving 
        const bool isEnd = !validIdxEntry(stackState.idx), hasElement = traverseState.defElementID > 0;
        [[flatten]] if (isEnd || hasElement) doIntersection( isEnd, hasElement, stateSwitched );
        [[flatten]] if (!validIdxIncluse(stackState.idx) || (!stateSwitched && (stackState.idx == traverseState.entryIDBase))) { break; };
    }; //while (validIdxIncluse(stackState.idx) && (stateSwitched || stackState.idx != traverseState.entryIDBase));
    return floatBitsToInt(primitiveState.lastIntersection.w);
};
