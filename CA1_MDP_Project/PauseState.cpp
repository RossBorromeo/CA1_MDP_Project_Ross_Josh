//Ross - D00241095 | Josh - D00238448
#include "PauseState.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include "Utility.hpp"

PauseState::PauseState(StateStack& stack, Context context)
    :State(stack, context)
    , m_background_sprite()
    , m_paused_text()
    , m_instruction_text()
    , m_esc_text()
{
    sf::Font& font = context.fonts->Get(Font::kMain);
    sf::Vector2f view_size = context.window->getView().getSize();

    m_paused_text.setFont(font);
    m_paused_text.setString("Paused");
    m_paused_text.setCharacterSize(70);
    Utility::CentreOrigin(m_paused_text);
    m_paused_text.setFillColor(sf::Color::Magenta);
    m_paused_text.setPosition(0.5f * view_size.x, 0.4f * view_size.y);

    m_instruction_text.setFont(font);
    m_instruction_text.setString("Backspace - Main Menu,");
    Utility::CentreOrigin(m_instruction_text);
    m_instruction_text.setFillColor(sf::Color::Magenta);
    m_instruction_text.setPosition(0.5f * view_size.x, 0.6f * view_size.y);

    m_esc_text.setFont(font);
    m_esc_text.setString("ESC - Play On");
    Utility::CentreOrigin(m_esc_text);
    m_esc_text.setFillColor(sf::Color::Magenta);
    m_esc_text.setPosition(0.5f * view_size.x, 0.7f * view_size.y);

    //Pause the music
    GetContext().music->SetPaused(true);
}

PauseState::PauseState(StateStack& stack, Context context, bool isNetworked)
    : State(stack, context)
    , m_background_sprite()
    , m_paused_text()
    , m_instruction_text()
    , m_esc_text()
{
    // You can use `isNetworked` to modify behavior
}

void PauseState::Draw()
{
    sf::RenderWindow& window = *GetContext().window;
    window.setView(window.getDefaultView());

    sf::RectangleShape backgroundShape;
    backgroundShape.setFillColor(sf::Color(0, 0, 0, 150));
    backgroundShape.setSize(window.getView().getSize());

    window.draw(backgroundShape);
    window.draw(m_paused_text);
    window.draw(m_instruction_text);
    window.draw(m_esc_text);
}

bool PauseState::Update(sf::Time dt)
{
    return false;
}

bool PauseState::HandleEvent(const sf::Event& event)
{
    if (event.type != sf::Event::KeyPressed)
    {
        return false;
    }

    if (event.key.code == sf::Keyboard::Escape)
    {
        RequestStackPop();
    }

    if (event.key.code == sf::Keyboard::BackSpace)
    {
        RequestStackClear();
        RequestStackPush(StateID::kMenu);
    }
    return false;
}

PauseState::~PauseState()
{
    GetContext().music->SetPaused(false);
}