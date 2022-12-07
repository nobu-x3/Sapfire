#pragma once
#include "Component.h"
#include <string>
#include <unordered_map>
namespace Patterns
{
	class StateMachine : public Component
	{
	      public:
		StateMachine(class Actor *owner);
		void Update(float deltaTime) override;
		void ProcessInput(const uint8_t *keyState) override;
		void RegisterState(class State *state);
		void ChangeState(const std::string &name);

	      private:
		std::unordered_map<std::string, class State *> mStateMap;
		class State *mCurrentState;
	};
} // namespace StateMachine
