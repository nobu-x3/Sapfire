#pragma once
#include <dxcapi.h>
#include <wrl.h>
#include "core/core.h"
#include "render/resources.h"

namespace Sapfire::tools::shader_compiler {

	struct SFAPI Shader {
		Microsoft::WRL::ComPtr<IDxcBlob> shader_blob{};
		Microsoft::WRL::ComPtr<IDxcBlob> root_signature_blob{};
	};


	Shader SFAPI compile(const d3d::ShaderType& type, const stl::string_view path, const stl::string_view entry_point,
						 const bool extract_root_signature = false);

	Shader SFAPI compile(const d3d::ShaderType& type, const stl::wstring_view path, const stl::wstring_view entry_point,
						 const bool extract_root_signature = false);

} // namespace Sapfire::tools::shader_compiler
