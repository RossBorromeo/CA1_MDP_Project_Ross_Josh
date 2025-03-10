//D00238448:Joshua Thompson
//d00241095:Ross Borromeo
#pragma once
#include "ResourceIdentifiers.hpp"
#include "StateID.hpp"
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <memory>
#include "MusicPlayer.hpp"
#include "SoundPlayer.hpp"

namespace sf
{
	class RenderWindow;
}

class Player;
class StateStack;
class KeyBinding;

class State
{
public:
	typedef std::unique_ptr<State> Ptr;

	struct Context
	{
		//added in by Josh to Handle 2 players

		Context(sf::RenderWindow& window, TextureHolder& textures, FontHolder& fonts, Player& player1, Player& player2, MusicPlayer& music, SoundPlayer& sounds);
		sf::RenderWindow* window;
		TextureHolder* textures;
		FontHolder* fonts;
		Player* player1;
		Player* player2;
		MusicPlayer* music;
		SoundPlayer* sounds;
		KeyBinding* keys1;
		KeyBinding* keys2;
	};

public:
	State(StateStack& stack, Context context);
	virtual ~State();
	virtual void Draw() = 0;
	virtual bool Update(sf::Time dt) = 0;
	virtual bool HandleEvent(const sf::Event& event) = 0;


protected:
	void RequestStackPush(StateID state_id);
	void RequestStackPop();
	void RequestStackClear();

	Context GetContext() const;

private:
	StateStack* m_stack;
	Context m_context;
};

