#include <iostream>

using namespace std;

class game_entity_base
{
public:
    game_entity_base(int id)
        : _id{id}
    {}

    virtual ~game_entity_base() = default;

public:
    int id() const { return _id; }

private:
    int _id;
};

template <typename Entity>
class state
{
public:
    virtual void enter(Entity& e) = 0;
    virtual void execute(Entity& e) = 0;
    virtual void exit(Entity& e) = 0;
};

template <typename Entity>
class generic_state_machine
{
public:
    using entity_state = state<Entity>;

public:
    generic_state_machine(Entity& owner,
            entity_state& global_state,
            entity_state& current_state)
        : _owner{&owner}
        , _global_state{&global_state}
        , _current_state{&current_state}
        , _previous_state{&current_state}
    {}

public:
    entity_state& global_state() const { return *_global_state; }
    entity_state& current_state() const { return *_current_state; }
    entity_state& previous_state() const { return *_previous_state; }

public:
    void set_current_state(entity_state& current_state)
    {
        _previous_state = _current_state;

        _current_state->exit(*_owner);

        _current_state = &current_state;

        _current_state->enter(*_owner);
    }

    void return_to_previous_state()
    {
        set_current_state(*_previous_state);
    }

private:
    Entity* _owner;

    entity_state* _global_state;
    entity_state* _current_state;
    entity_state* _previous_state;
};

class miner;
using miner_state = state<miner>;

class global_state : public miner_state
{
public:
    static global_state& instance()
    {
        static auto i = global_state{};

        return i;
    }

public:
    void enter(miner& m) override;
    void execute(miner& m) override;
    void exit(miner& m) override;
};

class peeing_state : public miner_state
{
public:
    static peeing_state& instance()
    {
        static auto i = peeing_state{};

        return i;
    }

public:
    void enter(miner& m) override;
    void execute(miner& m) override;
    void exit(miner& m) override;
};

class mining_state : public miner_state
{
public:
    static mining_state& instance()
    {
        static auto i = mining_state{};

        return i;
    }

public:
    void enter(miner& m) override;
    void execute(miner& m) override;
    void exit(miner& m) override;
};

class miner : public game_entity_base
{
public:
    static const auto bladder_volume = 10;

public:
    using miner_state_machine = generic_state_machine<miner>;

public:
    miner(int id)
        : game_entity_base{id}
        , _state_machine{*this, global_state::instance(), mining_state::instance()}
    {}

public:
    void update()
    {
        ++_current_bladder_volume;

        _state_machine.global_state().execute(*this);

        _state_machine.current_state().execute(*this);
    }

public:
    int current_bladder_volume() const { return _current_bladder_volume; }
    miner_state_machine& state_machine() { return _state_machine; }

public:
    void set_current_bladder_volume(int vol) { _current_bladder_volume = vol; }

private:
    miner_state_machine _state_machine;

private:
    int _current_bladder_volume = 0;
};

void global_state::enter(miner&)
{
    cout << "entering global state" << endl;
}

void global_state::execute(miner& m)
{
    if(m.current_bladder_volume() >= miner::bladder_volume)
        m.state_machine().set_current_state(peeing_state::instance());
}

void global_state::exit(miner&)
{
    cout << "exiting global state" << endl;
}

void peeing_state::enter(miner& m)
{
    cout << "peeing_state::enter(): miner[" << m.id() << "] unzipping to pee" << endl;
}

void peeing_state::execute(miner& m)
{
    cout << "peeing_state::execute(): miner[" << m.id() << "] returning to previous state" << endl;

    m.state_machine().return_to_previous_state();
}

void peeing_state::exit(miner& m)
{
    cout << "peeing_state::exit(): miner[" << m.id() << "] relieved" << endl;

    m.set_current_bladder_volume(0);
}

void mining_state::enter(miner& m)
{
    cout << "mining_state::enter(): miner[" << m.id() << "] preparing to mine" << endl;
}

void mining_state::execute(miner& m)
{
    cout << "mining_state::execute(): miner[" << m.id() << "] continuing to mine" << endl;
}

void mining_state::exit(miner& m)
{
    cout << "mining_state::exit(): miner[" << m.id() << "] stopped mining" << endl;
}

int main()
{
    auto miner1 = miner{1};

    for(auto i = miner::bladder_volume + 5; i > 0; --i)
        miner1.update();
}
