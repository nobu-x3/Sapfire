#include "engpch.h"
#include "RenderCommands.h"
#include "renderer/opengl/OpenGLRendererAPI.h"

RendererAPI *RenderCommands::sRendererAPI =
    new OpenGLRendererAPI(); // TODO: will actually be decided automatically at runtime
