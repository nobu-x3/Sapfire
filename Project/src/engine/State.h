class State
{
	public:
	State(class StateMachine *owner);
	virtual void Update(float deltaTime) = 0;
	virtual void OnEnter() = 0;
	virtual void OnExit() = 0;
	virtual const char *GetName() const = 0;

	protected:
	class StateMachine *mOwner;
};
