


// require solve problem with State registry ( probably, need even better hardware ) 
void traverseBVH2( in bool reset, in bool valid ) {
    //switchStateTo(BVH_STATE_TOP, 0, valid);
    valid = valid && validIdx(traverseState.idx);
    [[flatten]] if (valid) [[dependency_infinite]] for (uint hi=0;hi<maxIterations;hi++) {  // two loop based BVH traversing
        [[flatten]] if ( validIdx(traverseState.idx) ) {
        { [[dependency_infinite]] for (;hi<maxIterations;hi++) { bool _continue = false;
            //const NTYPE_ bvhNode = bvhNodes[traverseState.idx]; // each full node have 64 bytes
            #define bvhNode bvhNodes[traverseState.idx]
            const ivec2 cnode = validIdx(traverseState.idx) ? bvhNode.meta.xy : (0).xx;
            [[flatten]] if (isLeaf(cnode.xy)) { traverseState.defElementID = VTX_PTR + cnode.x; } // if leaf, defer for intersection 
            else { // if not leaf, intersect with nodes
                //const fmat3x4_ bbox2x = fmat3x4_(bvhNode.cbox[0], bvhNode.cbox[1], bvhNode.cbox[2]);

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
                pbool_ fmask = pl_x(childIntersect)|(pl_y(childIntersect)<<true_);
                traverseState.idx = -1;
                [[flatten]] if (fmask > 0) { _continue = true;
                    int secondary = -1; 
                    [[flatten]] if (fmask == 3) { fmask &= true_<<pbool_(nfe.x>nfe.y); secondary = cnode.x^int(fmask>>1u); }; // if both has intersection
                    traverseState.idx = traverseState.entryIDBase + (cnode.x^int(fmask&1u)); // set traversing node id

                    // pre-intersection that triangle, because any in-stack op can't check box intersection doubly or reuse
                    // also, can reduce useless stack storing, and make more subgroup friendly triangle intersections
                    //#define snode (bvhNodes[traverseState.entryIDBase+secondary].meta.xy) // use reference only
                    [[flatten]] if (secondary > 0) {
                        const ivec2 snode = bvhNodes[traverseState.entryIDBase+secondary].meta.xy;
                        [[flatten]] if (isLeaf(snode) && currentState == BVH_STATE_BOTTOM) { traverseState.defElementID = VTX_PTR + snode.x; secondary = -1; } else 
                        [[flatten]] if (secondary > 0) storeStack(traverseState.entryIDBase+secondary);
                    };
                };
            };

            // if all threads had intersection, or does not given any results, break for processing
            //[[flatten]] if ( !_continue && validIdx(traverseState.idx) ) { traverseState.idx = -1, loadStack(traverseState.idx); } // load from stack 
            [[flatten]] if ( !_continue ) { traverseState.idx = -1, loadStack(traverseState.idx); } // load from stack 
            [[flatten]] IFANY (traverseState.defElementID > 0 || !validIdx(traverseState.idx)) { break; }; // 
        }}};
        
        // every-step solving 
        //const int tlIdx = traverseState.idx;
        const uint CSTATE = currentState;
        [[flatten]] IFANY (traverseState.defElementID > 0) { doIntersection( valid ); }; // 
        [[flatten]] if (!validIdx(traverseState.idx)) {
            //const uint CSTATE = currentState;
            [[flatten]] if (CSTATE == BVH_STATE_BOTTOM) { switchStateTo(BVH_STATE_TOP, INSTANCE_ID, valid); };
            [[flatten]] if (CSTATE == BVH_STATE_TOP) break;
        };
    };
};
