#include "SettingsState.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"

#include <iostream>

SettingsState::SettingsState(StateStack& stack, Context context)
	: State(stack, context)
	, m_gui_container()
{
	m_background_sprite.setTexture(context.textures->Get(TextureID::kTitleScreen));

	

	// Player 1 Controls
	AddButtonLabel(Action::kMoveUp, 150.f, "Move Up P1", context, true);
	AddButtonLabel(Action::kMoveDown, 200.f, "Move Down P1", context, true);
	AddButtonLabel(Action::kMoveRight, 250.f, "Move Right P1", context, true);
	AddButtonLabel(Action::kMoveLeft, 300.f, "Move Left P1", context, true);
	AddButtonLabel(Action::kBulletFire, 350.f, "Fire P1", context, true);
	AddButtonLabel(Action::kMissileFire, 400.f, "Missile Fire P1", context, true);

	// Player 2 Controls
	AddButtonLabel(Action::kMoveUp1, 150.f, "Move Up P2", context, false);
	AddButtonLabel(Action::kMoveDown1, 200.f, "Move Down P2", context, false);
	AddButtonLabel(Action::kMoveRight1, 250.f, "Move Right P2", context, false);
	AddButtonLabel(Action::kMoveLeft1, 300.f, "Move Left P2", context, false);
	AddButtonLabel(Action::kBulletFire1, 350.f, "Fire P2", context, false);
	AddButtonLabel(Action::kMissileFire1, 400.f, "Missile Fire P2", context, false);

	UpdateLabels();

	auto back_button = std::make_shared<gui::Button>(context);
	back_button->setPosition(80.f, 475.f);
	back_button->SetText("Back");
	back_button->SetCallback(std::bind(&SettingsState::RequestStackPop, this));
	m_gui_container.Pack(back_button);
}

void SettingsState::Draw()
{
	sf::RenderWindow& window = *GetContext().window;
	window.draw(m_background_sprite);
	window.draw(m_gui_container);
}

bool SettingsState::Update(sf::Time dt)
{
	return true;
}

bool SettingsState::HandleEvent(const sf::Event& event)
{
	bool is_key_binding = false;

	// Iterate through all key binding buttons for Player 1 and Player 2
	for (std::size_t action = 0; action < static_cast<int>(Action::kActionCount); ++action)
	{
		if (m_binding_buttons[action] && m_binding_buttons[action]->IsActive())
		{
			is_key_binding = true;

			if (event.type == sf::Event::KeyReleased)
			{
				if (action <= static_cast<int>(Action::kMissileFire)) {
					GetContext().player1->AssignKey(static_cast<Action>(action), event.key.code);
				}
				else {
					GetContext().player2->AssignKey(static_cast<Action>(action), event.key.code);
				}
				m_binding_buttons[action]->Deactivate();
			}
			break;
		}
	}

	// If key bindings changed, update the labels
	if (is_key_binding)
	{
		UpdateLabels();
	}
	else
	{
		m_gui_container.HandleEvent(event);
	}
	return false;
}

void SettingsState::UpdateLabels()
{
	Player& player1 = *GetContext().player1;
	Player& player2 = *GetContext().player2;

	for (std::size_t i = 0; i < static_cast<int>(Action::kActionCount); ++i)
	{
		sf::Keyboard::Key key;
		if (i <= static_cast<int>(Action::kMissileFire)) {
			key = player1.GetAssignedKey(static_cast<Action>(i));
		}
		else {
			key = player2.GetAssignedKey(static_cast<Action>(i));
		}

		if (m_binding_labels[i]) {
			m_binding_labels[i]->SetText(Utility::toString(key));
		}
	}
}

void SettingsState::AddButtonLabel(Action action, float y, const std::string& text, Context context, bool isPlayer1)
{
	int actionIndex = static_cast<int>(action);

	// Ensure actionIndex is valid
	if (actionIndex < 0 || actionIndex >= static_cast<int>(Action::kActionCount)) {
		std::cerr << "ERROR: Invalid action index: " << actionIndex << std::endl;
		return;
	}

	// Create the button
	m_binding_buttons[actionIndex] = std::make_shared<gui::Button>(context);
	m_binding_buttons[actionIndex]->setPosition(isPlayer1 ? 80.f : 500.f, y);  // Player 1 on left, Player 2 on right
	m_binding_buttons[actionIndex]->SetText(text);
	m_binding_buttons[actionIndex]->SetToggle(true);

	// Create the label
	m_binding_labels[actionIndex] = std::make_shared<gui::Label>("", *context.fonts);
	m_binding_labels[actionIndex]->setPosition(isPlayer1 ? 300.f : 720.f, y + 15.f);  // Player 1 on left, Player 2 on right

	// Pack into GUI container
	m_gui_container.Pack(m_binding_buttons[actionIndex]);
	m_gui_container.Pack(m_binding_labels[actionIndex]);
}
