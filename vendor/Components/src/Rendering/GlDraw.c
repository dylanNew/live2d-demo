/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include <Live2DCubismGlRendering.h>


// -------- //
// REQUIRES //
// -------- //

#include "Local.h"

#include <Live2DCubismCore.h>
#include <Live2DCubismGlRenderingINTERNAL.h>


// ----- //
// TYPES //
// ----- //

/// Draw context.
typedef struct DrawContext
{
  /// Active renderer.
  const csmGlRenderer* Renderer;

  /// User matrix.
  const GLfloat* Mvp;

  /// Available textures.
  const GLuint* Textures;


  /// Currently active program.
  GlProgram ActiveProgram;

  /// Currently set blend mode.
  GLuint ActiveBlendMode;

  /// Currently set texture.
  GLuint ActiveTexture;

  /// Currently set opacity.
  float ActiveOpacity;

  /// Non-zero if culling is active.
  GLint IsCullingActive;
}
DrawContext;


// --------- //
// VARIABLES //
// --------- //

/// Blend scales table.
static GLenum BlendScale[3][4] =
{
  /// Normal blending.
  {GL_ONE , GL_ONE_MINUS_SRC_ALPHA , GL_ONE , GL_ONE_MINUS_SRC_ALPHA},

  /// Additive blending.
  {GL_SRC_ALPHA , GL_ONE , GL_ZERO , GL_ONE},

  /// Multiplicative blending.
  {GL_DST_COLOR , GL_ONE_MINUS_SRC_ALPHA , GL_ZERO , GL_ONE}
};


// --------- //
// FUNCTIONS //
// --------- //

/// Initializes a context.
///
/// @param  context   Context to initialize.
/// @param  renderer  Active renderer.
/// @param  mvp       User matrix.
/// @param  textures  Textures available for drawing.
static void InitializeDrawContext(DrawContext* context, const csmGlRenderer* renderer, const GLfloat* mvp, const GLuint* textures)
{
  // Initialize context.
  context->Renderer = renderer;
  context->Mvp = mvp;
  context->Textures = textures;


  context->ActiveProgram = 0;
  context->ActiveBlendMode = 3;
  context->ActiveTexture = 0;
  context->ActiveOpacity = -1.0f;
  context->IsCullingActive = -1;


  // Initialize OpenGL state.
  ActivateGlProgram(GlNonMaskedProgram);
  SetGlMvp(mvp);
}


/// Sets OpenGL states for a render drawable.
///
/// @param  context         Current draw context.
/// @param  renderDrawable  Drawable to set state for.
static void SetGlState(DrawContext* context, const csmRenderDrawable* renderDrawable)
{
  const csmRenderDrawable* mask;
  int d, m, cull, maskCount;
  GLuint maskTexture;
  GlProgram program;
  

  // Pick non-masked program as default.
  program = GlNonMaskedProgram;


  // Handle masking.
  d = (int)(renderDrawable - context->Renderer->RenderDrawables);
  maskCount = csmGetDrawableMaskCounts(context->Renderer->Model)[d];


  if (maskCount > 0)
  {
    // Set OpenGL state for drawing masks.
    // TODO  Only do this if necessary.
    context->ActiveProgram = GlMaskProgram;


    ActivateGlMaskbuffer();


    ActivateGlProgram(GlMaskProgram);
    SetGlMvp(context->Mvp);
    SetGlOpacity(context->ActiveOpacity);
    SetGlDiffuseTexture(context->ActiveTexture);


    // Enforce normal blending.
    glBlendFuncSeparate(BlendScale[0][0],
                        BlendScale[0][1],
                        BlendScale[0][2],
                        BlendScale[0][3]);


    // Draw masks.
    for (m = 0; m < maskCount; ++m)
    {
      mask = &context->Renderer->RenderDrawables[csmGetDrawableMasks(context->Renderer->Model)[d][m]];


      glDrawElements(GL_TRIANGLES,
                    csmGetRenderDrawableGlIndexCount(mask),
                    GL_UNSIGNED_SHORT,
                    csmGetRenderDrawableGlIndicesOffset(mask));
    }


    // Fetch mask texture and trigger program change.
    maskTexture = DeactivateGlMaskbuffer();
    program = GlMaskedProgram;
  }


  // Set program.
  if (context->ActiveProgram != program)
  {
    context->ActiveProgram = program;


    // Set program, matrix (, and mask texture).
    ActivateGlProgram(program);
    SetGlMvp(context->Mvp);


    if (program == GlMaskedProgram)
    {
      SetGlMaskTexture(maskTexture);
    }


    // Force refresh of other states.
    context->ActiveBlendMode = 3;
    context->ActiveTexture = 0;
    context->ActiveOpacity = -1.0f;
  }


  // Set diffuse texture.
  if (context->Textures[renderDrawable->TextureIndex] != context->ActiveTexture)
  {
    context->ActiveTexture = context->Textures[renderDrawable->TextureIndex];


    SetGlDiffuseTexture(context->ActiveTexture);
  }


  // Set blend state.
  if (renderDrawable->BlendMode != context->ActiveBlendMode)
  {
    context->ActiveBlendMode = renderDrawable->BlendMode;


    glEnable(GL_BLEND);


    glBlendFuncSeparate(BlendScale[context->ActiveBlendMode][0],
                        BlendScale[context->ActiveBlendMode][1],
                        BlendScale[context->ActiveBlendMode][2],
                        BlendScale[context->ActiveBlendMode][3]);
  }


  // Set opacity.
  if (renderDrawable->Opacity != context->ActiveOpacity)
  {
    context->ActiveOpacity = renderDrawable->Opacity;


    SetGlOpacity(context->ActiveOpacity);
  }


  // Set culling.
  cull = !renderDrawable->IsDoubleSided;


  if (cull != context->IsCullingActive)
  {
    context->IsCullingActive = cull;


    if (cull)
    {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
    }
    else
    {
      glDisable(GL_CULL_FACE);
    }
  }
}


// -------------- //
// IMPLEMENTATION //
// -------------- //

void csmGlDraw(csmGlRenderer* renderer, const GLfloat* mvp, const GLuint* textures)
{
  const RenderDrawable* renderDrawable;
  DrawContext context;
  int d;


  // Validate arguments.
  Ensure(renderer, "\"renderer\" is invalid.", return);
  Ensure(mvp, "\"mvp\" is invalid.", return);
  Ensure(textures, "\"textures\" are invalid.", return);
  Ensure((!renderer->IsBarebone), "\"renderer\" is barebone.", return);


  // Prepare context and with it GL states.
  InitializeDrawContext(&context, renderer, mvp, textures);


  // Bind geometry.
#if _CSM_COMPONENTS_USE_GL33
  glBindVertexArray(renderer->VertexArray);
#elif _CSM_COMPONENTS_USE_GLES20
  BindGlBuffer(&renderer->Buffers.Positions);
  glVertexAttribPointer(GetGlVertexPositionLocation(), 2, GL_FLOAT, GL_FALSE, 0, 0);


  BindGlBuffer(&renderer->Buffers.Uvs);
  glVertexAttribPointer(GetGlVertexUvLocation(), 2, GL_FLOAT, GL_FALSE, 0, 0);


  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);


  BindGlBuffer(&renderer->Buffers.Indices);
#endif


  // Draw.
  for (d = 0; d < renderer->DrawableCount; ++d)
  {
    // Fetch render drawable.
    renderDrawable = renderer->RenderDrawables + renderer->SortedDrawables[d].DrawableIndex;


    // Skip non-visible drawables.
    if (!renderDrawable->IsVisible)
    {
      continue;
    }


    // Update OpenGL state.
    SetGlState(&context, renderDrawable);


    // Draw geometry.
    glDrawElements(GL_TRIANGLES,
                   csmGetRenderDrawableGlIndexCount(renderDrawable),
                   GL_UNSIGNED_SHORT,
                   csmGetRenderDrawableGlIndicesOffset(renderDrawable));
  }


  // Unbind geometry.
#if _CSM_COMPONENTS_USE_GL33
  glBindVertexArray(0);
#elif _CSM_COMPONENTS_USE_GLES20
  UnbindGlBuffer(&renderer->Buffers.Uvs);
  UnbindGlBuffer(&renderer->Buffers.Indices);
#endif
}
