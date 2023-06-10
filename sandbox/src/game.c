#include "core/application.h"
#include "core/input.h"
#include "core/logger.h"
#include "core/sfmemory.h"
#include "game.h"
#include "math/sfmath.h"
// HACK: This should not be available outside the engine.
#include <renderer/renderer.h>

void recalculate_view_matrix (game_state* state) {
	if (state->camera_view_dirty) {
		mat4 rotation =
			mat4_euler_xyz (state->camera_euler.x, state->camera_euler.y,
							state->camera_euler.z);
		mat4 translation = mat4_translation (state->camera_position);

		state->view = mat4_mul (rotation, translation);
		state->view = mat4_inverse (state->view);

		state->camera_view_dirty = FALSE;
	}
}

void camera_yaw (game_state* state, f32 amount) {
	state->camera_euler.y += amount;
	state->camera_view_dirty = TRUE;
}

void camera_pitch (game_state* state, f32 amount) {
	state->camera_euler.x += amount;

	// Clamp to avoid Gimball lock.
	f32 limit			  = deg_to_rad (89.0f);
	state->camera_euler.x = CLAMP (state->camera_euler.x, -limit, limit);

	state->camera_view_dirty = TRUE;
}
b8 game_init (game* game_instance) {
	SF_DEBUG ("Game initialized");
	game_state* state = (game_state*)game_instance->state;

	state->camera_position = (vec3){0, 0, 0.0f};
	state->camera_euler	   = vec3_zero ();

	state->view				 = mat4_translation (state->camera_position);
	state->view				 = mat4_inverse (state->view);
	state->camera_view_dirty = TRUE;
	return TRUE;
}
b8 game_update (game* game_instance, f32 delta_time) {
	game_state* state = (game_state*)game_instance->state;
	application_state* app =
		(application_state*)game_instance->application_state;
	// HACK: temp hack to move camera around.
	if (input_is_key_down (KEY_A) || input_is_key_down (KEY_LEFT)) {
		camera_yaw (state, 1.0f * delta_time);
	}

	if (input_is_key_down (KEY_D) || input_is_key_down (KEY_RIGHT)) {
		camera_yaw (state, -1.0f * delta_time);
	}

	if (input_is_key_down (KEY_UP)) { camera_pitch (state, 1.0f * delta_time); }

	if (input_is_key_down (KEY_DOWN)) {
		camera_pitch (state, -1.0f * delta_time);
	}

	f32 temp_move_speed = 0.0f;
	vec3 velocity		= vec3_zero ();

	if (input_is_key_down (KEY_W)) {
		SF_DEBUG ("FORWARD");
		vec3 forward = mat4_forward (state->view);
		velocity	 = vec3_add (velocity, forward);
	}

	if (input_is_key_down (KEY_S)) {
		vec3 backward = mat4_backward (state->view);
		velocity	  = vec3_add (velocity, backward);
	}

	if (input_is_key_down (KEY_Q)) {
		vec3 left = mat4_left (state->view);
		velocity  = vec3_add (velocity, left);
	}

	if (input_is_key_down (KEY_E)) {
		vec3 right = mat4_right (state->view);
		velocity   = vec3_add (velocity, right);
	}

	if (input_is_key_down (KEY_SPACE)) { velocity.y += 1.0f; }

	if (input_is_key_down (KEY_X)) { velocity.y -= 1.0f; }

	vec3 z = vec3_zero ();
	if (!vec3_compare (z, velocity, 0.0002f)) {
		// Be sure to normalize the velocity before applying speed.
		vec3_normalize (&velocity);
		state->camera_position.x += velocity.x * temp_move_speed * delta_time;
		state->camera_position.y += velocity.y * temp_move_speed * delta_time;
		state->camera_position.z += velocity.z * temp_move_speed * delta_time;
		state->camera_view_dirty = TRUE;
	}

	recalculate_view_matrix (state);

	// HACK: This should not be available outside the engine.
	renderer_set_view (&app->renderer, state->view);
	return TRUE;
}
b8 game_render (game* game_instance, f32 deltaTime) { return TRUE; }
