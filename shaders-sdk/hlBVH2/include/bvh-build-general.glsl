#ifdef USE_MORTON_32
#define BIT_FFT 32
#define BIT_TPE uint
#else
#define BIT_FFT 64
#define BIT_TPE uvec2
#endif

int prefixOf( in int a, in int b ) {
    const BIT_TPE acode = Mortoncodes[a], bcode = Mortoncodes[b]; const int pfx = nlz(acode^bcode);
    return ((a >= 0 && b >= 0) && (a < GSIZE && b < GSIZE)) ? (pfx + (pfx < BIT_FFT ? 0 : nlz(a^b))) : -1;
}

int findSplit( in int left, in int right ) {
    const int commonPrefix = prefixOf(left, right);
    int split = left, nstep = right - left, nsplit = split + nstep;
    IFALL (commonPrefix >= BIT_FFT) { split = (left + right)>>1; };
    if    (commonPrefix  < BIT_FFT) {
        [[dependency_infinite]] do {
            nstep = (nstep + 1) >> 1, nsplit = split + nstep;
            [[flatten]] if (prefixOf(split, nsplit) > commonPrefix) { split = nsplit; }
        } while (nstep > 1);
    };
    return clamp(split, left, right-1);
}
/*
ivec2 determineRange( in int idx ) {
    const int dir = clamp(prefixOf(idx,idx+1) - prefixOf(idx,idx-1), -1, 1), minPref = prefixOf(idx,idx-dir); int maxLen = 2, len = 0;
    [[dependency_infinite]] while (prefixOf(idx, maxLen*dir+idx) > minPref) maxLen <<= 1;
    [[dependency_infinite]] for (int t = maxLen>>1; t > 0; t>>=1) { [[flatten]] if (prefixOf(idx, (len+t)*dir+idx) > minPref) len+= t; }

    const int range = len * dir + idx;
    return clamp(ivec2(min(idx,range), max(idx,range)), (0).xx, (GSIZE-1).xx);
}*/

// from top to bottom scheme (fine layout)
void splitNode(in int pID) {
    const ivec2 range = bvhNodes[pID].meta.xy-1;
    [[flatten]] if (range.x != range.y && range.x >= 0 && range.y >= 0 && range.y < GSIZE) {
        const int split = findSplit(range.x, range.y);
        const ivec4 transplit = ivec4(range.x, split+0, split+1, range.y);
        const bvec2 isLeaf = lessThan(transplit.yw - transplit.xz, ivec2(1,1));
        
        // resolve branch
        const ivec2 h = ((split+1)<<1).xx|ivec2(0,1);
        bvhNodes[pID].meta.xy = h.xy+1, 
        bvhNodes[h.x].meta = ivec4(transplit.xy+1, pID+1,0), 
        bvhNodes[h.y].meta = ivec4(transplit.zw+1, pID+1,0);

#ifdef USE_ACTIVE
        [[flatten]] if (any(not(isLeaf))) Actives[swapOut+wID(aCounterInc())] = h.x+1;
#endif

        // set leaf indices, without using atomics
        [[flatten]] if (isLeaf.x) { LeafIndices[split+0] = h.x+1; };
        [[flatten]] if (isLeaf.y) { LeafIndices[split+1] = h.y+1; };
    }
}
