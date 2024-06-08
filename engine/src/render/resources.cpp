#include "core/logger.h"
#include "engpch.h"

#include "render/resources.h"

namespace Sapfire::d3d {
	Allocation::Allocation(const Allocation& other) : allocation(other.allocation), resource(other.resource) {
		if (other.mapped_data.has_value()) {
			mapped_data = other.mapped_data;
		}
	}

	Allocation& Allocation::operator=(const Allocation& other) {
		if (&other == this)
			return *this;
		resource->Release();
		allocation->Release();
		if (other.mapped_data.has_value()) {
			mapped_data = other.mapped_data;
		}
		resource = other.resource;
		allocation = other.allocation;
		return *this;
	}

	Allocation::Allocation(Allocation&& other) noexcept : allocation(std::move(other.allocation)), resource(std::move(other.resource)) {
		if (other.mapped_data.has_value()) {
			mapped_data = other.mapped_data;
		}
	}

	Allocation& Allocation::operator=(Allocation&& other) noexcept {
		resource = std::move(other.resource);
		allocation = std::move(other.allocation);
		if (other.mapped_data.has_value()) {
			mapped_data = other.mapped_data;
		}
		return *this;
	}

	void Allocation::update(const void* data, size_t size) {
		PROFILE_FUNCTION();
		if (!data || !mapped_data.has_value()) {
			CORE_ERROR("Trying to update resource that is not placed in CPU-visible memory, or the data is null");
			return;
		}
		memcpy(mapped_data.value(), data, size);
	}

	void Allocation::reset() {
		PROFILE_FUNCTION();
		resource.Reset();
		allocation.Reset();
	}

	void Buffer::update(const void* data) {
		PROFILE_FUNCTION();
		allocation.update(data, size_in_bytes);
	}

	bool Texture::is_srgb(DXGI_FORMAT format) {
		switch (format) {
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			{
				return true;
			}
			break;
		default:
			{
				return false;
			}
			break;
		}
	}

	DXGI_FORMAT Texture::get_non_srgb_format(DXGI_FORMAT format) {
		switch (format) {
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			{
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			}
			break;

		case DXGI_FORMAT_BC1_UNORM_SRGB:
			{
				return DXGI_FORMAT_BC1_UNORM;
			}
			break;

		case DXGI_FORMAT_BC2_UNORM_SRGB:
			{
				return DXGI_FORMAT_BC2_UNORM;
			}
			break;

		case DXGI_FORMAT_BC3_UNORM_SRGB:
			{
				return DXGI_FORMAT_BC3_UNORM;
			}
			break;

		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			{
				return DXGI_FORMAT_B8G8R8A8_UNORM;
			}
			break;

		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			{
				return DXGI_FORMAT_B8G8R8X8_UNORM;
			}
			break;

		case DXGI_FORMAT_BC7_UNORM_SRGB:
			{
				return DXGI_FORMAT_BC7_UNORM;
			}
			break;

		default:
			{
				return format;
			}
			break;
		}
	}

} // namespace Sapfire::d3d
