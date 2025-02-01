#pragma once
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>

class StateStack;
class Player;
class FontHolder;
class SoundPlayer;
class TextureHolder;
class sf::RenderWindow;

class State
{
public:
	struct Context
	{
		Context(sf::RenderWindow& window, FontHolder& fonts, SoundPlayer& sounds, TextureHolder& textures, Player& player);

		sf::RenderWindow* window;
		FontHolder* fonts;
		SoundPlayer* sounds;
		TextureHolder* textures;
		Player* player;
	};

public:
	explicit State(StateStack& stack, Context context);
	virtual ~State() = default;

	virtual void Draw() = 0;
	virtual bool Update(sf::Time dt) = 0;
	virtual bool HandleEvent(const sf::Event& event) = 0;
	Context GetContext() const;

protected:
	void RequestStackPush(StateID stateID);
	void RequestStackPop();
	void RequestStackClear();

protected:
	StateStack* m_stack;
	Context m_context;
};
