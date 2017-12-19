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

// Cubism model.
typedef struct csmModel csmModel;


// OpenGL signed integer.
typedef int GLint;

// OpenGL unsigned short.
typedef unsigned short GLushort;

// OpenGL unsigned integer.
typedef unsigned int GLuint;

// OpenGL single-precision float.
typedef float GLfloat;

// OpenGL enum value.
typedef unsigned int GLenum;


// OpenGL (integer) size.
typedef int GLsizei;

// OpenGL void.
typedef void GLvoid;


// ----- //
// TYPES //
// ----- //

enum
{
  /// Normal blending mode.
  csmNormalBlending = 0,

  /// Additive blending mode.
  csmAdditiveBlending = 1,

  /// Multiplicative blending mode.
  csmMultiplicativeBlending = 2,
};


/// Abstraction layer for sorting drawables by their rendering order.
typedef struct csmSortableDrawable
{
  /// Initial index of the drawable.
  int DrawableIndex;

  /// Render order.
  int RenderOrder;
}
csmSortableDrawable;


/// Drawable data used by renderers.
typedef struct csmRenderDrawable
{
  /// Opacity.
  float Opacity;

  /// Texture index.
  unsigned short TextureIndex;

  /// Blend mode.
  unsigned short BlendMode : 14;

  /// Non-zero if double-sided.
  unsigned short IsDoubleSided : 1;

  /// Non-zero if visible.
  unsigned short IsVisible : 1;


  /// Vertex buffers information.
  struct
  {
    /// Offset into per-renderer buffer in 'types'.
    unsigned short BaseIndex;

    /// Number of buffered data in 'types'.
    unsigned short Count;
  }
  Vertices;


  /// Triangle index information.
  struct
  {
    /// Offset into per-renderer buffer in 'types'.
    unsigned short BaseIndex;

    /// Number of buffered data in 'types'.
    unsigned short Count;
  }
  Indices;
}
csmRenderDrawable;


/// OpenGL buffer abstraction.
typedef struct csmGlBuffer
{
  /// Buffer type.
  GLenum Type;

  /// Buffer handle.
  GLuint Handle;
}
csmGlBuffer;


/// OpenGL renderer.
typedef struct csmGlRenderer
{
  /// OpenGl buffers.
  struct
  {
    /// Vertex position buffer of OpenGL type 'vec2'.
    csmGlBuffer Positions;

    /// Vertex UV buffer of OpenGL type 'vec2'.
    csmGlBuffer Uvs;

    /// Triangle index buffer.
    csmGlBuffer Indices;
  }
  Buffers;


  /// Vertex array object. (Unused on OpenGLES 2.0).
  GLuint VertexArray;


  /// Non-zero if renderer is barebone, i.e. can't be builtin drawn.
  GLint IsBarebone : 1;


  /// Number of drawables.
  GLint DrawableCount : 31;

  /// Render drawables.
  csmRenderDrawable* RenderDrawables;

  /// Sorted drawables.
  csmSortableDrawable* SortedDrawables;


  /// Model to render.
  const csmModel* Model;  
}
csmGlRenderer;


// ---------------- //
// RENDER DRAWABLES //
// ---------------- //

/// TODO  Document.
///
/// @param  drawable  Drawable to query.
///
/// @return  Index count.
static inline GLsizei csmGetRenderDrawableGlIndexCount(const csmRenderDrawable* drawable)
{
  return (GLsizei)drawable->Indices.Count;
}

/// TODO  Document.
///
/// @param  drawable  Drawable to query.
///
/// @return  Offset pointer into index buffer.
static inline GLvoid* csmGetRenderDrawableGlIndicesOffset(const csmRenderDrawable* drawable)
{
  return (GLvoid*)(sizeof(GLushort) * drawable->Indices.BaseIndex);
}


// ----------- //
// GL RENDERER //
// ----------- //

/// Initializes a renderer for custom drawing.
///
/// @param  model                            Model to represent.
/// @param  address                          Address to place renderer at.
/// @param  size                             Size of memory block for instance (in bytes).
/// @param  vertexPositionAttributeLocation  Valid attribute location for the vertex positions, a 'vec2'.
/// @param  vertexUvAttributeLocation        Valid attribute location for the vertex uvs, also a 'vec2'.
///
/// @return  A valid pointer on success; '0' otherwise.
csmGlRenderer* csmMakeBareboneGlRendererInPlace(const csmModel* model,
						  				                   			  void* address,
			                                  			  const unsigned int size,
																								const GLint vertexPositionAttributeLocation,
																								const GLint vertexUvAttributeLocation);
