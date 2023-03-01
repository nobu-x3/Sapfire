#pragma once
#include "Sapfire/core/Input.h"
#include "Sapfire/scene/ScriptableEntity.h"

namespace Sapfire
{
	class CameraController : public ScriptableEntity
	{
	protected:
		void on_update(float deltaTime) override
		{
			{
				PROFILE_SCOPE("Inputs");
				mCameraRotation -= glm::vec3(Input::get_mouse_y(), Input::get_mouse_x(), 0.f) - mPrevCameraRot;
				mPrevCameraRot = {Input::get_mouse_y(), Input::get_mouse_x(), 0.f};
				if (Input::mouse_button_down(MouseButton::Right))
				{
					if (Input::key_pressed(KeyCode::A))
						mDirection += glm::vec3({-1, 0, 0});
					if (Input::key_pressed(KeyCode::D))
						mDirection += glm::vec3({1, 0, 0});
					if (Input::key_pressed(KeyCode::W))
						mDirection += glm::vec3({0, 0, -1});
					if (Input::key_pressed(KeyCode::S))
						mDirection += glm::vec3({0, 0, 1});
				}
			}
			{
				PROFILE_SCOPE("Gameplay");
				auto forwardVec = get_component<TransformComponent>().get_forward_vector();
				get_component<TransformComponent>().Translation += mDirection != glm::vec3(0)
					? forwardVec * MOVE_SPEED *
					deltaTime * mDirection.z
					: glm::vec3(0);
				if (Input::mouse_button_down(MouseButton::Right))
				{
					get_component<TransformComponent>().set_euler_rotation(
						get_component<TransformComponent>().get_euler_rotation() + mCameraRotation * deltaTime);
				}
				mCameraRotation = glm::vec3(0);
				mDirection = glm::vec3(0);
			}
		}

	private:
		glm::vec3 mDirection{0.f, 0.f, 0.f};
		glm::vec3 mCameraRotation{0.f, 0.f, 0.f};
		glm::vec3 mPrevCameraRot = {0.f, 0.f, 0.f};
		const float MOVE_SPEED = 150.f;
	};
}
