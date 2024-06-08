#include "Sapfire.h"
#include "imgui.h"

namespace icons {
    constexpr const char* MESH_ICON_64_ID = "mesh_icon_64";
    constexpr const char* MESH_ICON_16_ID = "mesh_icon_16";
    const Sapfire::d3d::Texture& get(const Sapfire::stl::string& id);
    ImTextureID get_im_id(const Sapfire::stl::string& id);
    void add(Sapfire::d3d::GraphicsDevice& device, const Sapfire::stl::wstring& path, const Sapfire::stl::string& id);
}
