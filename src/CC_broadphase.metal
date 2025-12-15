#include <metal_stdlib>
#include <metal_atomic>
using namespace metal;

struct AtomicCounter {
    atomic_uint value;
};
struct AABB_GPU {
    float4 min;
    float4 max;
};

struct CollisionPair {
    uint a;
    uint b;
};

kernel void broadphase(
    device const AABB_GPU* aabbs       [[buffer(0)]],
    device AtomicCounter*   pairCount   [[buffer(1)]],
    device CollisionPair* pairs       [[buffer(2)]],
    constant uint&        count       [[buffer(3)]],
    uint id [[thread_position_in_grid]]
){
    uint i = id/count;
    uint j = id % count;

    if(i >= j || j >= count) return;

    AABB_GPU A = aabbs[i];
    AABB_GPU B = aabbs[j];

    bool overlap =
        (A.min.x <= B.max.x && A.max.x >= B.min.x) &&
        (A.min.y <= B.max.y && A.max.y >= B.min.y) &&
        (A.min.z <= B.max.z && A.max.z >= B.min.z);

    if(overlap){
        uint index = atomic_fetch_add_explicit(&pairCount->value, 1, memory_order_relaxed);
        pairs[index] = { i, j };
    }
}