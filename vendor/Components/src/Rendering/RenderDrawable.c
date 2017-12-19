/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */
 
 
#include "Local.h"


// -------- //
// REQUIRES //
// -------- //

#include <Live2DCubismCore.h>
#include <Live2DCubismGlRenderingINTERNAL.h>


// -------------- //
// IMPLEMENTATION //
// -------------- //

void InitializeRenderDrawables(RenderDrawable* drawables, const csmModel* model)
{
  const int* vertexCounts, * indexCounts, * textureIndices;
  unsigned short vertexOffset, indexOffset;
  const unsigned char* constantFlags;
  int drawableCount, d;


  // Initialize locals.
  textureIndices = csmGetDrawableTextureIndices((csmModel*)model);
  constantFlags = csmGetDrawableConstantFlags((csmModel*)model);
  vertexCounts = csmGetDrawableVertexCounts((csmModel*)model);
  indexCounts = csmGetDrawableIndexCounts((csmModel*)model);

  drawableCount = csmGetDrawableCount((csmModel*)model);

  vertexOffset = 0;
  indexOffset = 0;


  // Initialize static drawable fields.
  for (d = 0; d < drawableCount; ++d)
  {
    drawables[d].TextureIndex = textureIndices[d];
    drawables[d].BlendMode = (IsBitSet(constantFlags[d], csmBlendAdditive))
      ? csmAdditiveBlending
      : (IsBitSet(constantFlags[d], csmBlendMultiplicative))
      ? csmMultiplicativeBlending
      : csmNormalBlending;
    drawables[d].IsDoubleSided = IsBitSet(constantFlags[d], csmIsDoubleSided);

    drawables[d].Vertices.BaseIndex = vertexOffset;
    drawables[d].Vertices.Count = (unsigned short)vertexCounts[d];

    drawables[d].Indices.BaseIndex = indexOffset;
    drawables[d].Indices.Count = (unsigned short)indexCounts[d];


    vertexOffset += drawables[d].Vertices.Count;
    indexOffset += drawables[d].Indices.Count;
  }
}
