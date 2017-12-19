/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */
 
 
 #include "Local.h"

 #include <stdlib.h>
 #include <stdio.h>


// ----- //
// TYPES //
// ----- //

/// OpenGL program abstraction layer.
typedef struct Program
{
  // Program handle.
  GLuint Handle;


  // Input locations.
  struct
  {
    /// Location of model-view-projection uniform.
    GLint Mvp;

    /// Location of opacity uniform.
    GLint Opacity;

    /// Location of mask texture.
    GLint MaskTexture;

    /// Location of diffuse texture.
    GLint DiffuseTexture;
  }
  Locations;
}
Program;


// ----------------- //
// SHADER CODE PARTS //
// ----------------- //

/// Vertex shader code.
static const GLchar VertexShaderCode[] =
#if _CSM_COMPONENTS_USE_GL33
"#version 330\n"


"in vec2 VertexPosition;"
"in vec2 VertexUv;"


"out vec4 Color;"
"out vec2 MaskUv;"
"out vec2 DiffuseUv;"


"uniform mat4 Mvp;"
"uniform float Opacity;"


"void main()"
"{"
"  vec4 position = Mvp * vec4(VertexPosition, 0.0, 1.0);"


"  vec4 screenPosition = position * 0.5;"


"  screenPosition.xy = screenPosition.xy + screenPosition.w;"
"  screenPosition.zw = position.zw;"


"  gl_Position = position;"


"  Color = vec4(1, 1, 1, Opacity);"
"  MaskUv = screenPosition.xy;"
"  DiffuseUv = VertexUv;"
"}";
#elif _CSM_COMPONENTS_USE_GLES20
"attribute vec2 VertexPosition;"
"attribute vec2 VertexUv;"


"varying vec4 Color;"
"varying vec2 MaskUv;"
"varying vec2 DiffuseUv;"


"uniform mat4 Mvp;"
"uniform float Opacity;"


"void main()"
"{"
"  vec4 position = Mvp * vec4(VertexPosition, 0.0, 1.0);"


"  vec4 screenPosition = position * 0.5;"


"  screenPosition.xy = screenPosition.xy + screenPosition.w;"
"  screenPosition.zw = position.zw;"


"  gl_Position = position;"


"  Color = vec4(1, 1, 1, Opacity);"
"  MaskUv = screenPosition.xy;"
"  DiffuseUv = VertexUv;"
"}";
#endif


/// Non-masked fragment shader code.
static const GLchar NonMaskedFragmentShaderCode[] =
#if _CSM_COMPONENTS_USE_GL33
"#version 330\n"


"in vec4 Color;"
"in vec2 DiffuseUv;"


"uniform sampler2D DiffuseTexture;"


"out vec4 FragColor;"


"void main()"
"{"
"  vec4 fragColor = texture(DiffuseTexture, DiffuseUv) * Color;"


"  fragColor.rbg *= fragColor.a;"


"  FragColor = fragColor;"
"}";
#elif _CSM_COMPONENTS_USE_GLES20
"varying mediump vec4 Color;"
"varying mediump vec2 DiffuseUv;"


"uniform sampler2D DiffuseTexture;"


"void main()"
"{"
"  mediump vec4 fragColor = texture2D(DiffuseTexture, DiffuseUv) * Color;"


"  fragColor.rbg *= fragColor.a;"


"  gl_FragColor = fragColor;"
"}";
#endif


/// Masked fragment shader code.
static const GLchar MaskedFragmentShaderCode[] =
#if _CSM_COMPONENTS_USE_GL33
"#version 330\n"
"in vec4 Color;"
"in vec2 MaskUv;"
"in vec2 DiffuseUv;"


"uniform sampler2D MaskTexture;"
"uniform sampler2D DiffuseTexture;"


"out vec4 FragColor;"


"void main()"
"{"
"  vec4 fragColor = texture(DiffuseTexture, DiffuseUv) * Color;"


"  fragColor.a *= texture(MaskTexture, MaskUv).a;"
"  fragColor.rbg *= fragColor.a;"


"  FragColor = fragColor;"
"}";
#elif _CSM_COMPONENTS_USE_GLES20
"varying mediump vec4 Color;"
"varying mediump vec2 MaskUv;"
"varying mediump vec2 DiffuseUv;"


"uniform sampler2D MaskTexture;"
"uniform sampler2D DiffuseTexture;"


"void main()"
"{"
"  mediump vec4 fragColor = texture2D(DiffuseTexture, DiffuseUv) * Color;"


"  fragColor.a *= texture2D(MaskTexture, MaskUv).a;"
"  fragColor.rbg *= fragColor.a;"


"  gl_FragColor = fragColor;"
"}";
#endif


/// Location of vertex position.
static const GLint VertexPositionLocation = 0;

/// Location of vertex UV.
static const GLint VertexUvLocation = 1;


// --------- //
// FUNCTIONS //
// --------- //

/// Creates a program.
///
/// @param  vertexShaderString        Vertex shader code.
/// @param  fragmentShaderString      Fragment shader code.
///
/// @return  Program.
static Program MakeProgram(const GLchar* vertexShaderString, const GLchar* fragmentShaderString)
{
  GLuint vertexShader, fragmentShader;
  Program program;


  // Make vertex shader.
  vertexShader = glCreateShader(GL_VERTEX_SHADER);


  glShaderSource(vertexShader, 1, &vertexShaderString, 0);
  glCompileShader(vertexShader);


  // Make fragment shader.
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);


  glShaderSource(fragmentShader, 1, &fragmentShaderString, 0);
  glCompileShader(fragmentShader);


  // Create program and attach shaders.
  program.Handle = glCreateProgram();


  glAttachShader(program.Handle, vertexShader);
  glAttachShader(program.Handle, fragmentShader);


  // Bind vertex attribute locations.
  glBindAttribLocation(program.Handle, VertexPositionLocation, "VertexPosition");
  glBindAttribLocation(program.Handle, VertexUvLocation, "VertexUv");


  // Link program.
  glLinkProgram(program.Handle);


  // Clean up shaders.
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);


  // Use program.
  glUseProgram(program.Handle);


  // Initialize locations.
  program.Locations.Mvp = glGetUniformLocation(program.Handle, "Mvp");
  program.Locations.Opacity = glGetUniformLocation(program.Handle, "Opacity");
  program.Locations.MaskTexture = glGetUniformLocation(program.Handle, "MaskTexture");
  program.Locations.DiffuseTexture = glGetUniformLocation(program.Handle, "DiffuseTexture");


  return program;
}

/// Releases OpenGL resources of a program.
///
/// @param  program  Program to release.
static void ReleaseProgram(Program* program)
{
  glDeleteProgram(program->Handle);
}


// --------- //
// VARIABLES //
// --------- //


/// Programs reference counter.
static int RetainPrograms = 0;

/// Programs.
static Program Programs[2] = {0};

/// Currently active program.
static int ActiveProgram = 0;


// -------------- //
// IMPLEMENTATION //
// -------------- //

void RequireGlPrograms()
{
  // Initialize programs if necessary.
  if (!RetainPrograms)
  {
    Programs[0] = MakeProgram(VertexShaderCode, NonMaskedFragmentShaderCode);
    Programs[1] = MakeProgram(VertexShaderCode, MaskedFragmentShaderCode);
  }


  ++RetainPrograms;
}

void UnrequireGlPrograms()
{
  int p;


  --RetainPrograms;


  // Release programs if appropriate.
  if (!RetainPrograms)
  {
    for (p = 0; p < (int)(sizeof(Programs) / sizeof(Programs[0])); ++p)
    {
      ReleaseProgram(&Programs[p]);
    }
  }
}


void ActivateGlProgram(GlProgram program)
{
  ActiveProgram = program % 2;


  glUseProgram(Programs[ActiveProgram].Handle);
}


GLint GetGlVertexPositionLocation()
{
  return VertexPositionLocation;
}

GLint GetGlVertexUvLocation()
{
  return VertexUvLocation;
}


void SetGlMvp(const GLfloat* value)
{
  glUniformMatrix4fv(Programs[ActiveProgram].Locations.Mvp, 1, GL_FALSE, value);
}

void SetGlOpacity(const float value)
{
  glUniform1f(Programs[ActiveProgram].Locations.Opacity, value);
}

void SetGlMaskTexture(const GLuint value)
{
	glUniform1i(Programs[ActiveProgram].Locations.MaskTexture, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, value);
}

void SetGlDiffuseTexture(const GLuint value)
{
	glUniform1i(Programs[ActiveProgram].Locations.DiffuseTexture, 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, value);
}
