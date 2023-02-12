#pragma once

#include <entt/entt.hpp>

 namespace Sapfire
 {
	 class Scene
	 {
	 public:
		 Scene();
		 ~Scene();

	 private:
		 entt::registry mRegistry;
	 };
 }