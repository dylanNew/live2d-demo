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

#if _CSM_COMPONENTS_USE_GL33
  #include _CSM_COMPONENTS_GL_H
#elif _CSM_COMPONENTS_USE_GLES20
  #if _CSM_COMPONENTS_IOS
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
  #else
    #include <GLES2/gl2.h>
  #endif
#endif


/// Cubism model.
typedef struct csmModel csmModel;


/// Render drawable.
typedef struct csmRenderDrawable RenderDrawable;

/// Sortable drawable.
typedef struct csmSortableDrawable SortableDrawable;


/// OpenGL buffer abstraction layer.
typedef struct csmGlBuffer GlBuffer;


/// Internal log function.
extern void Log(const char* message);


// ----- //
// TYPES //
// ----- //

/// OpenGL program handle.
typedef enum GlProgram
{
  /// Handle of OpenGL program for drawing masks.
  GlMaskProgram = 0,

  /// Handle of OpenGL program for drawing non-masked drawables.
  GlNonMaskedProgram = 2,

  /// Handle of OpenGL program for drawing masked drawables.
  GlMaskedProgram = 1
}
GlProgram;


// ---------- //
// ASSERTIONS //
// ---------- //

/// Ensures expression is valid.
///
/// @param  expression  Expression to validate.
/// @param  message     Message to log if validation fails.
/// @param  body        Body to execute if validation fails.
#define Ensure(expression, message, body)       \
do                                              \
{                                               \
  if (!(expression))                            \
  {                                             \
    Log("[Live2D Cubism Components] " message); \
    body;                                       \
  }                                             \
}                                               \
while (0);


// ----- //
// BYTES //
// ----- //

/// Checks whether a bit is set in a byte.
///
/// @param  byte  Byte to check.
/// @param  mask  Bit mask to apply.
///
/// @return  Non-zero if bit is set; '0' otherwise.
static inline int IsBitSet(const unsigned char byte, const unsigned char mask)
{
  return ((byte & mask) == mask);
}


// ---------------- //
// RENDER DRAWABLES //
// ---------------- //

// Initializes render drawables (without touching any GPU resources).
//
// @param  drawables  Drawables to initialize.
// @param  model      Model to reference.
void InitializeRenderDrawables(RenderDrawable* drawables, const csmModel* model);


// ------------------ //
// SORTABLE DRAWABLES //
// ------------------ //

/// Initializes sortable drawables.
///
/// @param  drawables  Drawables to initialize.
/// @param  model      Model to reference.
void InitializeSortableDrawables(SortableDrawable* drawables, const csmModel* model);


/// Updates render orders of sortable drawables and sorts them.
/// This is only necessary when one or more drawable render orders have changed. 
///
/// @param  drawables  Drawables to update and sort.
/// @param  model      Model to reference.
void UpdateSortableDrawables(SortableDrawable* drawables, const csmModel* model);


// ---------- //
// GL BUFFERS //
// ---------- //

/// Creates an unitialized OpenGL buffer flagged for use with static data.
///
/// @param  buffer  Buffer to initialize.
/// @param  type    Type of buffer to create.
/// @param  size    Size of the buffer in bytes.
void MakeStaticGlBufferInPlace(GlBuffer* buffer, const GLenum type, const GLsizeiptr size);

/// Creates an unitialized OpenGL buffer flagged for use with dynamic data.
///
/// @param  buffer  Buffer to initialize.
/// @param  type    Type of buffer to create.
/// @param  size    Size of the buffer in bytes.
void MakeDynamicGlBufferInPlace(GlBuffer* buffer, const GLenum type, const GLsizeiptr size);

// Frees GL buffer resources.
//
// @param  buffer  Buffer to release.
void ReleaseGlBuffer(GlBuffer* buffer);


// Binds an OpenGL buffer.
//
// @param  buffer  Buffer to bind.
void BindGlBuffer(GlBuffer* buffer);

// Unbinds an OpenGL buffer. (Actually unbinds any active buffer of matching type)...
//
// @param  buffer  Buffer to unbind.
void UnbindGlBuffer(GlBuffer* buffer);


// Writes data to an OpenGL buffer.
//
// @param  buffer           Buffer to write to.
// @param  offset           Offset bytes of writing start.
// @param  sizeofData       Size of data.
// @param  data             Source of data.
void WriteToGlBuffer(GlBuffer* buffer, const GLintptr offset, const GLsizeiptr sizeofData, const void* data);


// ----------- //
// GL PROGRAMS //
// ----------- //

/// Makes sure OpenGL programs are ready for use.
void RequireGlPrograms();

/// Allows OpenGL programs to be released.
void UnrequireGlPrograms();


/// Sets the OpenGL program to use.
void ActivateGlProgram(GlProgram program);


/// Gets the location of the OpenGL vertex position attribute.
///
/// @return  Location.
GLint GetGlVertexPositionLocation();

/// Gets the location of the OpenGL vertex uv attribute.
///
/// @return  Location.
GLint GetGlVertexUvLocation();


/// Sets the model-view-projection matrix for active program.
///
/// @param  value  4x4 matrix to apply.
void SetGlMvp(const GLfloat* mvp);

/// Sets opacity for active program.
///
/// @param  value  Value to set.
void SetGlOpacity(const float value);

/// Sets mask texture for active program.
///
/// @param  value  Value to set.
void SetGlMaskTexture(const GLuint value);

/// Sets diffuse texture for active program.
///
/// @param  value  Value to set.
void SetGlDiffuseTexture(const GLuint value);


// ------------- //
// GL MASKBUFFER //
// ------------- //

/// Makes sure OpenGL maskbuffer is ready for use.
void RequireGlMaskbuffer();

/// Allows OpenGL maskbuffer to be released.
void UnrequireGlMaskbuffer();


/// Sets up mask draw.
void ActivateGlMaskbuffer();

/// Finalizes mask draw.
///
/// @return  Mask texture drawn onto.
GLuint DeactivateGlMaskbuffer();
