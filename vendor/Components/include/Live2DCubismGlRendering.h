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


// OpenGL unsigned integer.
typedef unsigned int GLuint;

// OpenGL single-precision float.
typedef float GLfloat;


// ----- //
// TYPES //
// ----- //

/// Opaque OpenGL renderer of a modelfor rendering a model.
typedef struct csmGlRenderer csmGlRenderer;


// ----------- //
// GL RENDERER //
// ----------- //

/// Gets the necessary size for a renderable presentation of a model in bytes.
///
/// @param  model  Model to query for.
///
/// @return  Number of bytes necessary.
unsigned int csmGetSizeofGlRenderer(const csmModel* model);


/// Initializes a renderer for a model. This function creates OpenGL resources.
/// The calling thread must have a OpenGL context current.
///
/// Reallocating the model after initializing the renderer causes undefined behaviour.
///
/// @param  model    Model to represent.
/// @param  address  Address to place renderer at.
/// @param  size     Size of memory block for instance (in bytes).
///
/// @return  A valid pointer on success; '0' otherwise.
csmGlRenderer* csmMakeGlRendererInPlace(const csmModel* model,
						  				void* address,
			                            const unsigned int size);

/// Releases OpenGL renderer resources without touching user allocated memory.
/// The calling thread must have a OpenGL context current.
///
/// @param  renderer  Renderer to release.
void csmReleaseGlRenderer(csmGlRenderer* renderer);


/// Updates a renderer syncing it with its underlying model. The calling thread must have a GL context current.
///
/// Updating the underlying model while updating a renderer causes undefined behaviour.
///
/// @param  renderer  Renderer to update.
void csmUpdateGlRenderer(csmGlRenderer* renderer);


// ---------- //
// GL DRAWING //
// ---------- //

/// Draws a model. The calling thread must have an OpenGL context current.
///
/// @param  renderer  Model renderer.
/// @param  mvp       4x4 model-view-projection matrix.
/// @param  textures  Model texture(s).
void csmGlDraw(csmGlRenderer* renderer, const GLfloat* mvp, const GLuint* textures);
