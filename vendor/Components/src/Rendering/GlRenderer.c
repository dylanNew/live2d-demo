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


// --------- //
// FUNCTIONS //
// --------- //

// Converts from vertex data count to byte count.
//
// @param  count  Vertex data count.
//
// @return  Size of vertex data in bytes.
static inline GLsizeiptr ToSizeofVertexData(const unsigned short count)
{
  return sizeof(csmVector2) * count;
}

// Converts from index data count to byte count.
//
// @param  count  Index data count.
//
// @return  Size of index data in bytes.
static inline GLsizeiptr ToSizeofIndexData(const unsigned short count)
{
  return sizeof(unsigned short) * count;
}


/// Creates and initializes OpenGL buffers and vertex array.
/// Make sure to call this function AFTER non-OpenGL related renderer fields are initialized.
///
/// @param  renderer  Renderer to initialize.
static void InitializeBuffers(csmGlRenderer* renderer)
{
  int totalVertexCount, totalIndexCount, temporaryIndexBufferLength, d, i, j;
  unsigned short temporaryIndexBuffer[128];
  const int* vertexCounts, * indexCounts;
  RenderDrawable* renderDrawables;
  const unsigned short** indices;
  const csmVector2** vertexUvs;


  // Count model vertices and indices.
  totalVertexCount = 0;
  totalIndexCount = 0;


  vertexCounts = csmGetDrawableVertexCounts(renderer->Model);
  indexCounts = csmGetDrawableIndexCounts(renderer->Model);


  for (d = 0; d < renderer->DrawableCount; ++d)
  {
	  totalVertexCount += vertexCounts[d];
	  totalIndexCount += indexCounts[d];
  }


  // Create buffers.
  MakeDynamicGlBufferInPlace(&renderer->Buffers.Positions, GL_ARRAY_BUFFER, ToSizeofVertexData(totalVertexCount));
  MakeStaticGlBufferInPlace(&renderer->Buffers.Uvs, GL_ARRAY_BUFFER, ToSizeofVertexData(totalVertexCount));
  MakeStaticGlBufferInPlace(&renderer->Buffers.Indices, GL_ELEMENT_ARRAY_BUFFER, ToSizeofIndexData(totalIndexCount));


  // Initialize static buffers.
  renderDrawables = renderer->RenderDrawables;
  
  
  vertexUvs = csmGetDrawableVertexUvs(renderer->Model);
  BindGlBuffer(&renderer->Buffers.Uvs);


  for (d = 0; d < renderer->DrawableCount; ++d)
  {
    WriteToGlBuffer(&renderer->Buffers.Uvs, ToSizeofVertexData(renderDrawables[d].Vertices.BaseIndex), ToSizeofVertexData(renderDrawables[d].Vertices.Count), vertexUvs[d]);
  }


  UnbindGlBuffer(&renderer->Buffers.Uvs);


  // We store all vertices in one large buffer.
  // As 'glDrawElementsBaseVertex()' is pretty new on mobile, we patch vertex indices by hand here.
  temporaryIndexBufferLength = (int)(sizeof(temporaryIndexBuffer) / sizeof(temporaryIndexBuffer[0]));


  indices = csmGetDrawableIndices(renderer->Model);


  BindGlBuffer(&renderer->Buffers.Indices);


  i = 0;
  j = 0;


  for (d = 0; d < renderer->DrawableCount; ++d)
  {
   Loop:


    while (j < temporaryIndexBufferLength)
    {
      if (i >= renderDrawables[d].Indices.Count)
      {
        break;
      }
      

      temporaryIndexBuffer[j] = indices[d][i] + renderDrawables[d].Vertices.BaseIndex;;


      ++j;
      ++i;
    }


    WriteToGlBuffer(&renderer->Buffers.Indices, ToSizeofIndexData((i - j) + renderDrawables[d].Indices.BaseIndex), ToSizeofIndexData(j), temporaryIndexBuffer);
    
    
    j = 0;


    if (i < renderDrawables[d].Indices.Count)
    {
      goto Loop;
    }


    i = 0;
  }


  UnbindGlBuffer(&renderer->Buffers.Indices);
}


/// Creates and initializes OpenGL vertex array.
/// Make sure to call this function AFTER buffers are initialized.
///
/// @param  renderer                         Renderer to initialize.
/// @param  vertexPositionAttributeLocation  Attribute location for the vertex positions.
/// @param  vertexUvAttributeLocation        Attribute location for the vertex uvs.
#if _CSM_COMPONENTS_USE_GL33
static void InitializeVertexArray(csmGlRenderer* renderer,
                                  const GLint vertexPositionAttributeLocation,
                                  const GLint vertexUvAttributeLocation)
{
  // Create and initialize vertex array.
  glGenVertexArrays(1, &renderer->VertexArray);
  glBindVertexArray(renderer->VertexArray);


  BindGlBuffer(&renderer->Buffers.Positions);
  glVertexAttribPointer(vertexPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);


  BindGlBuffer(&renderer->Buffers.Uvs);
  glVertexAttribPointer(vertexUvAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);


  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);


  BindGlBuffer(&renderer->Buffers.Indices);


  // Unbind resources.
  glBindVertexArray(0);
  UnbindGlBuffer(&renderer->Buffers.Uvs);
  UnbindGlBuffer(&renderer->Buffers.Indices);
}
#endif


// -------------- //
// IMPLEMENTATION //
// -------------- //

unsigned int csmGetSizeofGlRenderer(const csmModel* model)
{
  // Validate arguments.
  Ensure(model, "\"model\" is invalid.", return 0);


	return (unsigned int)(sizeof(csmGlRenderer) + ((sizeof(csmRenderDrawable) + sizeof(csmSortableDrawable)) * csmGetDrawableCount(model)));
}


csmGlRenderer* csmMakeGlRendererInPlace(const csmModel* model,
										                    void* address,
										                    const unsigned int size)
{
  csmGlRenderer* renderer;


  // Validate arguments.
  Ensure(model, "\"model\" is invalid.", return 0);
  Ensure(address, "\"address\" is invalid.", return 0);  
  Ensure((size >= csmGetSizeofGlRenderer(model)), "\"size\" is invalid.", return 0);


  // Acquire resources.
  RequireGlPrograms();
  RequireGlMaskbuffer();


  // First initialize as barebone...
  renderer = csmMakeBareboneGlRendererInPlace(model,
                                              address,
                                              size,
                                              GetGlVertexPositionLocation(),
                                              GetGlVertexUvLocation());


  // ... then flag as non-barebone.
  renderer->IsBarebone = 0;


  return renderer;
}

csmGlRenderer* csmMakeBareboneGlRendererInPlace(const csmModel* model,
						  							                  	void* address,
			                                        	const unsigned int size,
																								const GLint vertexPositionAttributeLocation,
																								const GLint vertexUvAttributeLocation)
{
	csmGlRenderer* renderer;


  // Validate arguments.
  Ensure(model, "\"model\" is invalid.", return 0);
  Ensure(address, "\"address\" is invalid.", return 0);  
  Ensure((size >= csmGetSizeofGlRenderer(model)), "\"size\" is invalid.", return 0);


  renderer = (csmGlRenderer*)address;


  // Initialize non-OpenGL related fields.
  renderer->DrawableCount = csmGetDrawableCount(model);
  renderer->RenderDrawables = (csmRenderDrawable*)(renderer + 1);
  renderer->SortedDrawables = (csmSortableDrawable*)(renderer->RenderDrawables + renderer->DrawableCount);
  renderer->Model = model;


  InitializeRenderDrawables(renderer->RenderDrawables, model);
  InitializeSortableDrawables(renderer->SortedDrawables, model);


  // Initialize OpenGL resources and related drawables.
  InitializeBuffers(renderer);
#if _CSM_COMPONENTS_USE_GL33
  InitializeVertexArray(renderer, GetGlVertexPositionLocation(), GetGlVertexUvLocation());
#endif


	// Finalize initialization by calling update once.
	csmUpdateGlRenderer(renderer);


	// Return on success.
	return renderer;
}

void csmReleaseGlRenderer(csmGlRenderer* renderer)
{
  // Validate arguments.
  Ensure(renderer, "\"renderer\" is invalid.", return);


	// Release GL resources.
	ReleaseGlBuffer(&renderer->Buffers.Indices);
	ReleaseGlBuffer(&renderer->Buffers.Uvs);
	ReleaseGlBuffer(&renderer->Buffers.Positions);


  // Release draw-resources unless barebone.
  if (!renderer->IsBarebone)
  {
    UnrequireGlMaskbuffer();
    UnrequireGlPrograms();
  }
}


void csmUpdateGlRenderer(csmGlRenderer* renderer)
{
  const csmVector2** vertexPositions;
  const unsigned char* dynamicFlags;
  RenderDrawable* renderDrawables;
  const float* opacities;
  int d, sort;

  
  // Validate arguments.
  Ensure(renderer, "\"renderer\" is invalid.", return);


  // Initialize locals.
  vertexPositions = csmGetDrawableVertexPositions(renderer->Model);
  dynamicFlags = csmGetDrawableDynamicFlags(renderer->Model);
  opacities = csmGetDrawableOpacities(renderer->Model);

  renderDrawables = renderer->RenderDrawables;

  sort = 0;


  // Fetch dynamic data.
  BindGlBuffer(&renderer->Buffers.Positions);


  for (d = 0; d < renderer->DrawableCount; ++d)
  {
    // Update 'inexpensive' data without checking flags.
    renderDrawables[d].IsVisible = IsBitSet(dynamicFlags[d], csmIsVisible);	
    renderDrawables[d].Opacity = opacities[d];


    // Do expensive updates only if necessary.
    if (IsBitSet(dynamicFlags[d], csmVertexPositionsDidChange))
    {
      WriteToGlBuffer(&renderer->Buffers.Positions, ToSizeofVertexData(renderDrawables[d].Vertices.BaseIndex), ToSizeofVertexData(renderDrawables[d].Vertices.Count), vertexPositions[d]);
    }


    // Check whether drawables need to be sorted.
    sort = sort || IsBitSet(dynamicFlags[d], csmRenderOrderDidChange);
  }


  UnbindGlBuffer(&renderer->Buffers.Positions);


  // Do sort if necessary.
  if (sort)
  {
    UpdateSortableDrawables(renderer->SortedDrawables, renderer->Model);
  }
}
