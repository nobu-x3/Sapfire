IncludeDir = {}
IncludeDir["glew"] = "%{wks.location}/Engine/deps/glew/include"
IncludeDir["SDL2"] = "%{wks.location}/Engine/deps/SDL/include"
IncludeDir["spdlog"] = "%{wks.location}/Engine/deps/spdlog/include"
IncludeDir["assimp"] = "%{wks.location}/Engine/deps/assimp/include"
IncludeDir["glm"] = "%{wks.location}/Engine/deps/glm/"

Libs = {}
Libs["SDL2"] = "%{wks.location}/Engine/deps/SDL/VisualC/x64/Release/SDL2.lib"
Libs["spdlog"] = "%{wks.location}/Engine/deps/spdlog/build/spdlog.lib"
Libs["glew"] = "%{wks.location}/Engine/deps/glew/lib/Release/x64/glew32s.lib"
Libs["assimp"] = "%{wks.location}/Engine/deps/assimp/build/lib/Release/assimp-vc143-mt.lib"

