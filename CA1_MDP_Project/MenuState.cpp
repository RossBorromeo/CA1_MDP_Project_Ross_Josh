//Ross - D00241095 | Josh - D00238448
#include "MenuState.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Button.hpp"

//Ross - Adjusted Button to centre of the screen / added Host and Join buttons
MenuState::MenuState(StateStack& stack, Context context)
    :State(stack, context)
{
    sf::Texture& texture = context.textures->Get(TextureID::kTitleScreen);

    m_background_sprite.setTexture(texture);

    auto play_button = std::make_shared<gui::Button>(context);
    play_button->setPosition(860, 450);
    play_button->SetText("Play");
    play_button->SetCallback([this]()
        {
            RequestStackPop();
            RequestStackPush(StateID::kGame);
        });

    auto settings_button = std::make_shared<gui::Button>(context);
    settings_button->setPosition(860, 500);
    settings_button->SetText("Settings");
    settings_button->SetCallback([this]()
        {
            RequestStackPush(StateID::kSettings);
        });

    auto host_button = std::make_shared<gui::Button>(context); 
    host_button->setPosition(860, 550);
    host_button->SetText("Host");
    host_button->SetCallback([this]()
        {
            RequestStackClear(); // removes MenuState
            RequestStackPush(StateID::kHostGame);
        });

    auto join_button = std::make_shared<gui::Button>(context); 
    join_button->setPosition(860, 600);
    join_button->SetText("Join");
    join_button->SetCallback([this]()
        {
			RequestStackClear(); // removes MenuState
            RequestStackPush(StateID::kJoinGame);
        });

    auto exit_button = std::make_shared<gui::Button>(context);
    exit_button->setPosition(860, 650);
    exit_button->SetText("Exit");
    exit_button->SetCallback([this]()
        {
            RequestStackPop();
        });

    m_gui_container.Pack(play_button);
    m_gui_container.Pack(settings_button);
    m_gui_container.Pack(host_button);  
    m_gui_container.Pack(join_button);  
    m_gui_container.Pack(exit_button);

    //Play the music
    context.music->Play(MusicThemes::kMenuTheme);
}

void MenuState::Draw()
{
    sf::RenderWindow& window = *GetContext().window;
    window.setView(window.getDefaultView());
    window.draw(m_background_sprite);
    window.draw(m_gui_container);
}

bool MenuState::Update(sf::Time dt)
{
    return true;
}

bool MenuState::HandleEvent(const sf::Event& event)
{
    m_gui_container.HandleEvent(event);
    return true;
}

