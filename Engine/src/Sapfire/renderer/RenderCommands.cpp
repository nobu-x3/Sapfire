#include "engpch.h"
#include "RenderCommands.h"
#include "Sapfire/renderer/opengl/OpenGLRendererAPI.h"

namespace Sapfire
{
	RendererAPI* RenderCommands::sRendererAPI =
		new OpenGLRendererAPI(); // TODO: will actually be decided automatically at runtime
}