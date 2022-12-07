namespace Patterns
{
	class State
	{
	      public:
		State(class StateMachine *owner) : mOwner(owner) {}
		virtual void Update(float deltaTime) = 0;
		virtual void OnEnter();
		virtual void OnExit();
		virtual const char *GetName() const = 0;

	      private:
		class StateMachine *mOwner;
	};
} // namespace StateMachine
