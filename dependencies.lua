IncludeDir = {}
IncludeDir["glew"] = "%{wks.location}/Engine/deps/glew/include"
IncludeDir["SDL2"] = "%{wks.location}/Engine/deps/SDL/include"
IncludeDir["spdlog"] = "%{wks.location}/Engine/deps/spdlog/include"
IncludeDir["Assimp"] = "%{wks.location}/Engine/deps/assimp/include"
IncludeDir["glm"] = "%{wks.location}/Engine/deps/glm/"
IncludeDir["Glad"] = "%{wks.location}/Engine/deps/Glad/include"
IncludeDir["GLFW"] = "%{wks.location}/Engine/deps/GLFW/include"
IncludeDir["stb_image"] = "%{wks.location}/Engine/deps/stb_image"

Libs = {}
Libs["SDL2"] = "%{wks.location}/Engine/deps/SDL/VisualC/x64/Release/SDL2.lib"
-- Libs["spdlog"] = "%{wks.location}/Engine/deps/spdlog/build/spdlog.lib"
Libs["glew"] = "%{wks.location}/Engine/deps/glew/lib/Release/x64/glew32s.lib"
Libs["SOIL2"] = "%{wks.location}/Engine/deps/SOIL2/lib/windows/soil2.lib"
Libs["Assimp_Debug"] = "%{wks.location}/Engine/deps/assimp/bin/Debug/assimp-vc143-mtd.lib"
Libs["Assimp_Release"] = "%{wks.location}/Engine/deps/assimp/bin/Release/assimp-vc143-mt.lib"

Binaries = {}
Binaries["Assimp_Debug"] = "%{wks.location}/Engine/deps/assimp/bin/Debug/assimp-vc143-mtd.dll"
Binaries["Assimp_Release"] = "%{wks.location}/Engine/deps/assimp/bin/Release/assimp-vc143-mt.dll"