#include "Application.hpp"
#include "GameState.hpp"
#include "TitleState.hpp"
#include "MenuState.hpp"
#include "PauseState.hpp"
#include "SettingsState.hpp"
#include "GameOverState.hpp"
#include "MultiplayerGameState.hpp"

const sf::Time Application::kTimePerFrame = sf::seconds(1.f / 60.f);

Application::Application()
    : m_window(sf::VideoMode::getDesktopMode(), "Networked", sf::Style::Resize) //Ross - Size now resizes to your screen size
    , m_key_binding_1(1), m_key_binding_2(2)
    , m_stack(State::Context(m_window, m_textures, m_fonts, m_music, m_sound, m_key_binding_1, m_key_binding_2))
{
    m_window.setKeyRepeatEnabled(false);

    // Set view to match initial window size
    sf::Vector2u window_size = m_window.getSize();
    m_view.setSize(static_cast<float>(window_size.x), static_cast<float>(window_size.y));
    m_view.setCenter(window_size.x / 2.f, window_size.y / 2.f);
    m_window.setView(m_view);

    // Load resources
    m_fonts.Load(Font::kMain, "Media/Fonts/PixeloidSansBold-PKnYd.ttf");
    m_textures.Load(TextureID::kTitleScreen, "Media/Textures/TitleScreen.png");
    m_textures.Load(TextureID::kButtons, "Media/Textures/Buttons.png");

    RegisterStates();
    m_stack.PushState(StateID::kTitle);
}

void Application::Run()
{
    sf::Clock clock;
    sf::Time time_since_last_update = sf::Time::Zero;

    while (m_window.isOpen())
    {
        time_since_last_update += clock.restart();
        while (time_since_last_update > kTimePerFrame)
        {
            time_since_last_update -= kTimePerFrame;
            ProcessInput();
            Update(kTimePerFrame);

            if (m_stack.IsEmpty())
            {
                m_window.close();
            }
        }
        Render();
    }
}

void Application::ProcessInput()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        // Handle window resizing
        if (event.type == sf::Event::Resized)
        {
            sf::Vector2u new_size = m_window.getSize();
            m_view.setSize(static_cast<float>(new_size.x), static_cast<float>(new_size.y));
            m_view.setCenter(new_size.x / 2.f, new_size.y / 2.f);
            m_window.setView(m_view);
        }

        // Handle closing the window
        if (event.type == sf::Event::Closed)
        {
            m_window.close();
        }

        m_stack.HandleEvent(event);
    }
}

void Application::Update(sf::Time dt)
{
    m_stack.Update(dt);
}

void Application::Render()
{
    m_window.clear();
    m_stack.Draw();
    m_window.display();
}

void Application::RegisterStates()
{
    m_stack.RegisterState<TitleState>(StateID::kTitle);
    m_stack.RegisterState<MenuState>(StateID::kMenu);
    m_stack.RegisterState<GameState>(StateID::kGame);
    m_stack.RegisterState<MultiplayerGameState>(StateID::kHostGame, true);
    m_stack.RegisterState<MultiplayerGameState>(StateID::kJoinGame, false);
    m_stack.RegisterState<PauseState>(StateID::kPause);
    m_stack.RegisterState<PauseState>(StateID::kNetworkPause, true);
    m_stack.RegisterState<SettingsState>(StateID::kSettings);
    m_stack.RegisterState<GameOverState>(StateID::kGameOver, "Ship Destroyed!");
    m_stack.RegisterState<GameOverState>(StateID::kMissionSuccess, "Asteroids Avoided!");
}
