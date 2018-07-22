#ifdef USE_MORTON_32
#define BIT_FFT 32
#define BIT_TPE uint
#else
#define BIT_FFT 64
#define BIT_TPE uvec2
#endif

int cdelta( in int a, in int b ) {
    BIT_TPE acode = Mortoncodes[a], bcode = Mortoncodes[b]; int pfx = nlz(acode^bcode);
    return ((a >= 0 && b >= 0) && (a < GSIZE && b < GSIZE)) ? (pfx + (pfx < BIT_FFT ? 0 : nlz(a^b))) : -1;
}

int findSplit( in int left, in int right ) {
    int split = left, nstep = right - left, nsplit = split + nstep, commonPrefix = cdelta(split, nsplit);
    [[dependency_infinite]] do {
        nstep = (nstep + 1) >> 1, nsplit = split + nstep;
        if (cdelta(split, nsplit) > commonPrefix) { split = nsplit; }
    } while (nstep > 1);
    return clamp(split, left, right-1);
}

ivec2 determineRange( in int idx ) {
    int dir = clamp(cdelta(idx,idx+1) - cdelta(idx,idx-1), -1, 1), minPref = cdelta(idx,idx-dir), maxLen = 2;
    [[dependency_infinite]] while (cdelta(idx, maxLen*dir+idx) > minPref) maxLen <<= 1;

    int len = 0;
    [[dependency_infinite]] 
    for (int t = maxLen>>1; t > 0; t>>=1) { if (cdelta(idx, (len+t)*dir+idx) > minPref) len += t; }

    int range = len * dir + idx;
    return clamp(ivec2(min(idx,range), max(idx,range)), (0).xx, (GSIZE-1).xx);
}


// remove begin bit
int RFB(in int val){ return ((val>>1)<<1); }
bool ODDB(in int val){ return (val&1)==1; } // check bit is odd

// only low level, without hierarchies
void splitNode2(in int gID) {
    const int leafOffset = RFB(GSIZE+1);

    // pID is upper "split"
    // that "split" is unknown
    // any two children node index (offset) is equal split*2 + 2|3
    // it every two node contain range [??,split][split+1,??], at that index
    // evens has [??,split], odds is [split+1,??]
    // known one of range, but unknown a porential side
    // i.e. "split+1" may in range.x or "split" may in range.y, but we doesn't know, odd or even  

    //const bool ISODD = ??;
    //const int pID = RFB(ISODD ? range.x-1 : range.y) + (ISODD ? 1 : 0);
    //const ivec2 range = bvhMeta[pID].xy-1; // originally by from top to bottom
    const ivec2 range = determineRange(gID);
    [[flatten]]
    if (range.x >= 0 && range.y >= 0 && range.y < GSIZE) {
        [[flatten]]
        if (range.x != range.y) {
            const int split = findSplit(range.x, range.y); // split between childrens 
            const ivec4 transplit = ivec4(range.x, split+0, split+1, range.y);
            const bvec2 isLeaf = lessThan(transplit.yw - transplit.xz, ivec2(1,1));
            const ivec2 h = mix((0).xx, leafOffset.xx, isLeaf) + split.xx + ivec2(0,1);
            //const ivec2 h = ((split+1)<<1).xx + ivec2(0,1); // required pID, but it can't be calculated without full building from top to bottom

            // connect with child nodes 
            bvhMeta[gID].xy = h+1;
            bvhMeta[h.x].zw = ivec2(gID+1, 0);
            bvhMeta[h.y].zw = ivec2(gID+1, 0);

            // set leaf indices, without using atomics
            [[flatten]] if (isLeaf.x) { bvhMeta[h.x].xy = (split+1).xx; LeafIndices[split+0] = h.x+1; }
            [[flatten]] if (isLeaf.y) { bvhMeta[h.y].xy = (split+2).xx; LeafIndices[split+1] = h.x+2; }
        }
    }
}

// from top to bottom scheme (fine layout)
void splitNode(in int pID) {
    const ivec2 range = bvhMeta[pID].xy-1;
    [[flatten]]
    if (range.x >= 0 && range.y >= 0 && range.y < GSIZE) {
        [[flatten]]
        if (range.x != range.y) {
            const int split = findSplit(range.x, range.y);
            const ivec4 transplit = ivec4(range.x, split+0, split+1, range.y);
            const bvec2 isLeaf = lessThan(transplit.yw - transplit.xz, ivec2(1,1));
            
            // resolve branch
            const ivec2 h = ((split+1)<<1).xx + ivec2(0,1);
            bvhMeta[pID].xy = h+1;
            bvhMeta[h.x].zw = ivec2(pID+1,0), bvhMeta[h.y].zw = ivec2(pID+1,0);
            bvhMeta[h.x].xy = transplit.xy+1, bvhMeta[h.y].xy = transplit.zw+1;
            Actives[swapOut+wID(aCounterInc())] = h.x+1;
            
            //if (any(not(isLeaf))) Actives[swapOut+wID(aCounterInc())] =  h.x+1;
            //[[flatten]]
            //    if (all(isLeaf)) { const int cID = cCounterIncBoth(); LeafIndices[cID+0] = h.x+1, LeafIndices[cID+1] = h.y+1; } else
            //    if (any(isLeaf)) { LeafIndices[cCounterInc()] = (isLeaf.x ? h.x : h.y)+1; } 
        } else { LeafIndices[cCounterInc()] = pID+1; }
    }
}
