#pragma once
////////// CORE //////////////////
#include "core/application.h"
#include "core/core.h"
#include "core/game_context.h"
#include "core/layer.h"
#include "core/layer_stack.h"
#include "core/logger.h"
#include "core/memory.h"
#include "core/rtti.h"
#include "core/uuid.h"
/////////////////////////////////

///////// STL ///////////////////
#include "core/stl/unique_ptr.h"
/////////////////////////////////

///////// COMPONENTS ////////////
#include "components/component.h"
#include "components/ec_manager.h"
#include "components/entity.h"
#include "components/name_component.h"
#include "components/transform.h"
#include "components/render_component.h"
#include "components/test_custom_component.h"
/////////////////////////////////

//////// ASSETS /////////////////
#include "assets/mesh_manager.h"
#include "assets/texture_manager.h"
/////////////////////////////////

//////// EVENTS /////////////////
#include "events/event.h"
/////////////////////////////////

/////// TOOLS ///////////////////
#include "tools/profiling.h"
/////////////////////////////////
/////// RENDERING ///////////////
#include "render/camera.h"
#include "render/command_queue.h"
#include "render/context.h"
#include "render/d3d_primitives.h"
#include "render/d3d_util.h"
#include "render/descriptor_heap.h"
#include "render/graphics_context.h"
#include "render/graphics_device.h"
#include "render/lights.h"
#include "render/material.h"
#include "render/memory_allocator.h"
#include "render/pipeline_state.h"
#include "render/resources.h"
#include "render/upload_buffer.h"
#include "render/window.h"
/////////////////////////////////
