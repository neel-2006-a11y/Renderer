#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#include "CC_broadphase.h"
#include "MetalBroadPhase.h"
#include <vector>
#include <iostream>

static id<MTLDevice> device = nil;
static id<MTLCommandQueue> queue = nil;
static id<MTLComputePipelineState> pipeline = nil;

struct AtomicCounter{
    uint32_t value;
};
id<MTLBuffer> pairBuf;
void initMetalBroadPhase(int count) {
    std::cout << "Initializing Metal BroadPhase..." << std::endl;
    device = MTLCreateSystemDefaultDevice();
    queue = [device newCommandQueue];

    NSString* libPath = [[NSBundle mainBundle] pathForResource:@"CC_broadphase" ofType:@"metallib"];
    if(!libPath) {
        std::cerr << "Failed to find metallib path." << std::endl;
        return;
    }

    NSError* error = nil;
    id<MTLLibrary> lib = [device newLibraryWithFile:libPath error:&error];
    // NSLog(@"Metal BroadPhase library: %@", lib);
    NSArray *names = lib.functionNames;
    // NSLog(@"Metal BroadPhase functions: %@", names);
    id<MTLFunction> fn = [lib newFunctionWithName:@"broadphase"];
    
    pairBuf = 
        [device newBufferWithLength:sizeof(CollisionPair) * count * count
                           options:MTLResourceStorageModeShared];

    pipeline = [device newComputePipelineStateWithFunction:fn error:&error];
    if (!pipeline) {
        std::cerr << "Failed to create compute pipeline state: "
                  << [[error localizedDescription] UTF8String] << std::endl;
    } else {
        std::cout << "Metal BroadPhase initialized successfully." << std::endl;
    }
}

std::vector<CollisionPair> runBroadPhaseGPU(
    const std::vector<AABB_GPU>& aabbs
){
    uint32_t count = (uint32_t)aabbs.size();
    if(count < 2) return {};
    
    id<MTLBuffer> aabbBuf = 
        [device newBufferWithBytes:aabbs.data()
                            length:sizeof(AABB_GPU) * count
                           options:MTLResourceStorageModeShared];
    
    // id<MTLBuffer> pairBuf =
    //     [device newBufferWithLength:sizeof(CollisionPair) * count * count
    //                        options:MTLResourceStorageModeShared];
    AtomicCounter zero = {0};
    id<MTLBuffer> pairCountBuf =
        [device newBufferWithBytes:&zero
                            length:sizeof(AtomicCounter)
                           options:MTLResourceStorageModeShared];

    id<MTLCommandBuffer> cmd = [queue commandBuffer];
    id<MTLComputeCommandEncoder> enc = [cmd computeCommandEncoder];

    [enc setComputePipelineState:pipeline];
    [enc setBuffer:aabbBuf offset:0 atIndex:0];
    [enc setBuffer:pairCountBuf offset:0 atIndex:1];
    [enc setBuffer:pairBuf offset:0 atIndex:2];
    [enc setBytes:&count length:sizeof(uint32_t) atIndex:3];

    NSUInteger threads = count * count;
    NSUInteger tgSize = pipeline.maxTotalThreadsPerThreadgroup;

    [enc dispatchThreads:MTLSizeMake(threads, 1, 1)
      threadsPerThreadgroup:MTLSizeMake(tgSize, 1, 1)];

    [enc endEncoding];
    [cmd commit];
    [cmd waitUntilCompleted];

    uint32_t pairCount = ((AtomicCounter*)pairCountBuf.contents)->value;
    CollisionPair* data = (CollisionPair*)pairBuf.contents;

    return std::vector<CollisionPair>(data, data + pairCount);
}