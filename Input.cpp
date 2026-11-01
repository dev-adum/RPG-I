#include "Input.h"

void Input::BeginFrame()
{
    // Clear discrete presses; they are only valid for the frame they happen on.
    m_menu = false;
    m_fullscreen = false;
    m_escape = false;
    m_diceTest = false;
    m_reset = false;

    // Firing edge detection: fire is true only on the frame the button
    // went from up to down.
    const bool fireDown =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
        sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    m_firing = fireDown && !m_fireWasDown;
    m_fireWasDown = fireDown;
}

void Input::HandleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        switch (key->code)
        {
        case sf::Keyboard::Key::M:
            m_menu = true;
            break;
        case sf::Keyboard::Key::F11:
            m_fullscreen = true;
            break;
        case sf::Keyboard::Key::Escape:
            m_escape = true;
            break;
        case sf::Keyboard::Key::F:
            m_diceTest = true;
            break;
        case sf::Keyboard::Key::R:
            m_reset = true;
            break;
        default:
            break;
        }
    }
}

sf::Vector2f Input::GetMovement()
{
    sf::Vector2f dir(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) dir.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dir.x += 1.f;

    // Sprint while Shift is held
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift))
    {
        dir *= sprintMultiplier;
    }

    return dir;
}

bool Input::IsHoldingDice() const
{
    // Held rather than edge-triggered: the magnet is a state, not a press.
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E) ||
           sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
}

bool Input::IsCameraOrbiting() const
{
    return sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
}
