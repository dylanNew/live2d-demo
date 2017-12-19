/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once


// -------- //
// REQUIRES //
// -------- //

#if _USE_GLAD
  #include <glad/glad.h>
#elif _USE_GLES20
  #if _IOS
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
  #else
    #include <GLES2/gl2.h>
  #endif
#endif

// ---------- //
// ALLOCATION //
// ---------- //

/// Allocates heap memory without user align constraint.
///
/// @param  size   Number of bytes to allocate.
///
/// @return  Valid address to allocated memory on success; '0' otherwise.
void* Allocate(const unsigned int size);

/// Frees non-user alignment constraint heap memory.
///
/// @param  memory  Address of memory block to free.
void Deallocate(void* memory);


/// Allocates aligned heap memory.
///
/// @param  size       Number of bytes to allocate.
/// @param  alignment  Alignment for memory block.
///
/// @return  Valid address to allocated memory on success; '0' otherwise.
void* AllocateAligned(const unsigned int size, const unsigned int alignment);

/// Frees aligned heap memory.
///
/// @param  memory  Address of memory block to free.
void DeallocateAligned(void* alignedMemory);


