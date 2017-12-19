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

#include <stdlib.h>


// --------- //
// FUNCTIONS //
// --------- //

// Compares two sortable drawables by their render order.
//
// @param[in]  a  First drawable.
// @param[in]  b  First drawable.
//
// @return 'qsort()' compatible value.
static int CompareSortableDrawables(const void *a, const void *b)
{
  const SortableDrawable* drawableA = (const SortableDrawable*)a;
  const SortableDrawable* drawableB = (const SortableDrawable*)b;


  return (drawableA->RenderOrder > drawableB->RenderOrder) - (drawableA->RenderOrder < drawableB->RenderOrder);
}


// -------------- //
// IMPLEMENTATION //
// -------------- //

void InitializeSortableDrawables(SortableDrawable* drawables, const csmModel* model)
{
  int d, count;


  // Initialize static fields.
  for (d = 0, count = csmGetDrawableCount(model); d < count; ++d)
  {
    drawables[d].DrawableIndex = d;
  }
}

void UpdateSortableDrawables(SortableDrawable* drawables, const csmModel* model)
{
  const int* renderOrders;
  int d, count;


  renderOrders = csmGetDrawableRenderOrders(model);


  // Fetch render orders.
  for (d = 0, count = csmGetDrawableCount(model); d < count; ++d)
  {
    drawables[d].RenderOrder = renderOrders[drawables[d].DrawableIndex];
  }


  // Sort.
  qsort(drawables, count, sizeof(SortableDrawable), CompareSortableDrawables);
}
