//
//  Live2dHelper.m
//  live2d-demo
//
//  Created by Dylan Wang on 19/12/2017.
//  Copyright © 2017 Chaoxin. All rights reserved.
//

#import "Live2dHelper.h"
#include <Live2DCubismCore.h>
#include <Live2DCubismFramework.h>
#include <Live2DCubismGlRendering.h>
#include "Allocation.h"
#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

// ------- //
// HELPERS //
// ------- //

/// Logs message.
///
/// @param  message  Message to log.
static void DylanLog(const char* message)
{
    printf("%s\n", message);
}


/// Initializes matrix.
///
/// @param  vp            Matrix to initialize.
/// @param  windowWidth   Window width in pixels.
/// @param  windowHeight  Window height in pixels.
/// @param  safeFrame     Minimum units that should be visible.
static void InitializeVp(GLfloat* vp, float windowWidth, float windowHeight, float safeFrame)
{
    float aspect, xScale, yScale;
    
    
    // Compute scale.
    aspect = windowWidth / windowHeight;
    
    
    if (aspect > 1.0f)
    {
        yScale = (1.0f / safeFrame) * 2.0f;
        xScale = yScale * (1.0f / aspect);
    }
    else
    {
        aspect = windowHeight / windowWidth;
        
        
        xScale = (1.0f / safeFrame) * 2.0f;
        yScale = xScale * (1.0f / aspect);
    }
    
    
    // Set matrix.
    vp[0]  = xScale; vp[1]  = 0.0f;   vp[2]  = 0.0f; vp[3]  = 0.0f;
    vp[4]  = 0.0f;   vp[5]  = yScale; vp[6]  = 0.0f; vp[7]  = 0.0f;
    vp[8]  = 0.0f;   vp[9]  = 0.0f;   vp[10] = 1.0f; vp[11] = 0.0f;
    vp[12] = 0.0f;   vp[13] = 0.0f;   vp[14] = 0.0f; vp[15] = 1.0f;
}


// -------------- //
// IMPLEMENTATION //
// -------------- //

/// View projection matrix.
static GLfloat Vp[16];


const GLfloat* GetViewProjectionMatrix()
{
    return Vp;
}

@interface Live2dHelper() {
    csmMoc *moc;
    csmModel *model;
    csmModelHashTable *table;
    csmAnimation *animation;
    csmAnimationState animationState;
    csmGlRenderer *render;
    GLuint texture;
}
@end

@implementation Live2dHelper
- (instancetype)init {
    self = [super init];
    if (self) {
        [self initailize];
    }
    return self;
}

- (void)initailize {
    csmSetLogFunction(DylanLog);
    CGSize windowSize = [UIScreen mainScreen].bounds.size;
    InitializeVp(Vp, windowSize.width, windowSize.height, 1.0);
    
    unsigned int mocSize, modelSize, tableSize, renderSize;
    void *mocMemory = [self readMocMemoryOfsize:&mocSize];
    moc = csmReviveMocInPlace(mocMemory, mocSize);
    
    modelSize = csmGetSizeofModel(moc);
    void *modelMemory = AllocateAligned(modelSize, csmAlignofModel);
    model = csmInitializeModelInPlace(moc, modelMemory, modelSize);
    
    tableSize = csmGetSizeofModelHashTable(model);
    void *tableMemory = malloc(tableSize);
    table = csmInitializeModelHashTableInPlace(model, tableMemory, tableSize);
    
    const void *motionJson = [self getDataByName:@"Koharu.motion3" ofType:@"json"];
    const unsigned int animationSize = csmGetDeserializedSizeofAnimation(motionJson);
    void *animationMemory = malloc(animationSize);
    animation = csmDeserializeAnimationInPlace(motionJson, animationMemory, animationSize);
    
    csmResetAnimationState(&animationState);
    
    renderSize = csmGetSizeofGlRenderer(model);
    void *renderMemory = malloc(renderSize);
    render = csmMakeGlRendererInPlace(model, renderMemory, renderSize);
    
    texture = [self loadTextureFromPng];
}

- (const void *)getDataByName:(NSString *)name ofType:(NSString *)type {
    NSString *path = [[NSBundle mainBundle] pathForResource:name ofType:type];
    NSData *data = [[NSData alloc] initWithContentsOfFile:path];
    return [data bytes];
}

- (void *)readMocMemoryOfsize:(unsigned int*)outSize {
    void* blob;
    NSString *path = [[NSBundle mainBundle] pathForResource:@"Koharu" ofType:@"moc3"];
    NSData *data = [[NSData alloc] initWithContentsOfFile:path];
    unsigned int size = (unsigned int)data.length;
    
    
    // Allocate aligned memory for moc.
    blob = AllocateAligned(size, csmAlignofMoc);
    [data getBytes:blob length:data.length];
    
    // Store size.
    if (outSize) {
        (*outSize) = size;
    }
    
    return blob;
}

- (GLuint)loadTextureFromPng {
//    NSString *path = [[NSBundle mainBundle] pathForResource:@"Koharu" ofType:@"png"];
//    //Create texture
//    NSError *theError;
//    GLKTextureInfo *texture = [GLKTextureLoader textureWithContentsOfFile:path options:nil error:&theError];
//    glBindTexture(texture.target, texture.name);
//    return texture.name;
    UIImage *image = [UIImage imageNamed:@"Koharu.png"];
    return [self setupTexture:image];
}

/**
 http://blog.csdn.net/icetime17/article/details/50993655
 */
- (GLuint)setupTexture:(UIImage *)image {
    CGImageRef cgImageRef = [image CGImage];
    GLuint width = (GLuint)CGImageGetWidth(cgImageRef);
    GLuint height = (GLuint)CGImageGetHeight(cgImageRef);
    CGRect rect = CGRectMake(0, 0, width, height);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    void *imageData = malloc(width * height * 4);
    CGContextRef context = CGBitmapContextCreate(imageData, width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1.0f, -1.0f);
    CGColorSpaceRelease(colorSpace);
    CGContextClearRect(context, rect);
    CGContextDrawImage(context, rect, cgImageRef);
    
    glEnable(GL_TEXTURE_2D);
    
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    
   
    glBindTexture(GL_TEXTURE_2D, 0);
    CGContextRelease(context);
    free(imageData);
    
    return textureID;
}

- (void)onTick:(NSTimeInterval)duration {
    csmUpdateAnimationState(&animationState, duration);
    csmEvaluateAnimationFAST(animation, &animationState, csmOverrideFloatBlendFunction, 1.0f, model, table, 0, 0);
    
    csmUpdateModel(model);
    csmUpdateGlRenderer(render);
    csmResetDrawableDynamicFlags(model);
    
    csmGlDraw(render, Vp, &texture);
}

- (void)dealloc {
    // TODO: clean up
}
@end
