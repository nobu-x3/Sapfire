#include "RenderCommands.h"
#include "engine/engpch.h"
#include "engine/renderer/opengl/OpenGLRendererAPI.h"

RendererAPI *RenderCommands::sRendererAPI =
    new OpenGLRendererAPI(); // TODO: will actually be decided automatically at runtime
