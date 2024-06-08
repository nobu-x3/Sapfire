#include "engpch.h"

#include "core/file_system.h"
#include "core/logger.h"
#include "render/d3d_util.h"
#include "tools/shader_compiler.h"

namespace Sapfire::tools::shader_compiler {

	using namespace Microsoft::WRL;

	// Responsible for the actual compilation of shaders.
	ComPtr<IDxcCompiler3> compiler{};

	// Used to create include handle and provides interfaces for loading shader
	// to blob, etc.
	ComPtr<IDxcUtils> utils{};
	ComPtr<IDxcIncludeHandler> includeHandler{};

	std::wstring shader_directory{};

	Shader compile(const d3d::ShaderType& type, const stl::string_view path, const stl::string_view entry_point,
				   const bool extract_root_signature /*= false*/) {
		Shader shader{};
		if (!utils) {
			d3d_check(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
			d3d_check(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
			d3d_check(utils->CreateDefaultIncludeHandler(&includeHandler));
			shader_directory = fs::FileSystem::get_full_path(L"assets/shaders");
			CORE_INFO("Shader base directory: {}", d3d::WStringToANSI(shader_directory));
		}
		CORE_INFO("Compiling shader at path: {}", path);
		std::wstring shader_path = d3d::AnsiToWString(stl::string{path});
		// Setup compilation arguments.
		const std::wstring target_profile = [=]() {
			switch (type) {
			case d3d::ShaderType::Vertex:
				{
					return L"vs_6_6";
				}
				break;
			case d3d::ShaderType::Pixel:
				{
					return L"ps_6_6";
				}
				break;
			case d3d::ShaderType::Compute:
				{
					return L"cs_6_6";
				}
				break;
			default:
				{
					return L"";
				}
				break;
			}
		}();
		std::wstring entry = d3d::AnsiToWString(stl::string{entry_point});
		stl::vector<LPCWSTR> compilation_arguments = {
			L"-HV",
			L"2021",
			L"-E",
			entry.data(),
			L"-T",
			target_profile.c_str(),
			DXC_ARG_PACK_MATRIX_ROW_MAJOR,
			DXC_ARG_WARNINGS_ARE_ERRORS,
			DXC_ARG_ALL_RESOURCES_BOUND,
			L"-I",
			shader_directory.c_str(),
#if defined(DEBUG) | defined(_DEBUG)
			DXC_ARG_DEBUG
#else
			DXC_ARG_OPTIMIZATION_LEVEL3
#endif
		};
		// Load the shader source file to a blob.
		ComPtr<IDxcBlobEncoding> source_blob{nullptr};
		d3d_check(utils->LoadFile(shader_path.data(), nullptr, &source_blob));
		const DxcBuffer sourceBuffer = {
			.Ptr = source_blob->GetBufferPointer(),
			.Size = source_blob->GetBufferSize(),
			.Encoding = 0u,
		};
		// Compile the shader.
		ComPtr<IDxcResult> compiled_shader_buffer{};
		const HRESULT hr =
			compiler->Compile(&sourceBuffer, compilation_arguments.data(), static_cast<uint32_t>(compilation_arguments.size()),
							  includeHandler.Get(), IID_PPV_ARGS(&compiled_shader_buffer));
		if (FAILED(hr)) {
			CORE_CRITICAL("Failed to compile shader with path : {}", path);
		}
		// Get compilation errors (if any).
		ComPtr<IDxcBlobUtf8> errors{};
		d3d_check(compiled_shader_buffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
		if (errors && errors->GetStringLength() > 0) {
			const LPCSTR errorMessage = errors->GetStringPointer();
			CORE_CRITICAL("Shader path : {}, Error : {}", path, errorMessage);
		}
		ComPtr<IDxcBlob> compiled_shader_blob{nullptr};
		compiled_shader_buffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiled_shader_blob), nullptr);
		shader.shader_blob = compiled_shader_blob;
		ComPtr<IDxcBlob> root_signature_blob{nullptr};
		if (extract_root_signature) {
			compiled_shader_buffer->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&root_signature_blob), nullptr);
			shader.root_signature_blob = root_signature_blob;
		}
		return shader;
	}

	Shader compile(const d3d::ShaderType& type, const stl::wstring_view path, const stl::wstring_view entry,
				   const bool extract_root_signature) {
		Shader shader{};
		if (!utils) {
			d3d_check(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
			d3d_check(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
			d3d_check(utils->CreateDefaultIncludeHandler(&includeHandler));
			shader_directory = fs::FileSystem::get_full_path(L"assets/shaders");
			CORE_INFO("Shader base directory: {}", d3d::WStringToANSI(shader_directory));
		}
        stl::wstring full_path = shader_directory + L"/" + stl::wstring{path};
        std::string path_str = d3d::WStringToANSI(full_path);
		CORE_INFO("Compiling shader at path: {}", path_str);
		// Setup compilation arguments.
		const std::wstring target_profile = [=]() {
			switch (type) {
			case d3d::ShaderType::Vertex:
				{
					return L"vs_6_6";
				}
				break;
			case d3d::ShaderType::Pixel:
				{
					return L"ps_6_6";
				}
				break;
			case d3d::ShaderType::Compute:
				{
					return L"cs_6_6";
				}
				break;
			default:
				{
					return L"";
				}
				break;
			}
		}();
		stl::vector<LPCWSTR> compilation_arguments = {
			L"-HV",
			L"2021",
			L"-E",
			entry.data(),
			L"-T",
			target_profile.c_str(),
			DXC_ARG_PACK_MATRIX_ROW_MAJOR,
			DXC_ARG_WARNINGS_ARE_ERRORS,
			DXC_ARG_ALL_RESOURCES_BOUND,
			L"-I",
			shader_directory.c_str(),
#if defined(DEBUG) | defined(_DEBUG)
			DXC_ARG_DEBUG
#else
			DXC_ARG_OPTIMIZATION_LEVEL3
#endif
		};
		// Load the shader source file to a blob.
		ComPtr<IDxcBlobEncoding> source_blob{nullptr};
		d3d_check(utils->LoadFile(full_path.data(), nullptr, &source_blob));
		const DxcBuffer sourceBuffer = {
			.Ptr = source_blob->GetBufferPointer(),
			.Size = source_blob->GetBufferSize(),
			.Encoding = 0u,
		};
		// Compile the shader.
		ComPtr<IDxcResult> compiled_shader_buffer{};
		const HRESULT hr =
			compiler->Compile(&sourceBuffer, compilation_arguments.data(), static_cast<uint32_t>(compilation_arguments.size()),
							  includeHandler.Get(), IID_PPV_ARGS(&compiled_shader_buffer));
		if (FAILED(hr)) {
			CORE_CRITICAL("Failed to compile shader with path : {}", path_str);
		}
		// Get compilation errors (if any).
		ComPtr<IDxcBlobUtf8> errors{};
		d3d_check(compiled_shader_buffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
		if (errors && errors->GetStringLength() > 0) {
			const LPCSTR errorMessage = errors->GetStringPointer();
			CORE_CRITICAL("Shader path : {}, Error : {}", path_str, errorMessage);
		}
		ComPtr<IDxcBlob> compiled_shader_blob{nullptr};
		compiled_shader_buffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiled_shader_blob), nullptr);
		shader.shader_blob = compiled_shader_blob;
		ComPtr<IDxcBlob> root_signature_blob{nullptr};
		if (extract_root_signature) {
			compiled_shader_buffer->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&root_signature_blob), nullptr);
			shader.root_signature_blob = root_signature_blob;
		}
		return shader;
	}

} // namespace Sapfire::tools::shader_compiler
