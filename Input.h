#pragma once
#include <SFML/Graphics.hpp>

// Single source of truth for keyboard/mouse input.
//
// Two kinds of input live here, because they need different mechanisms:
//
//  * Continuous ("is it held right now?"): WASD, Shift, fire button.
//    Read with isKeyPressed / isMouseButtonPressed at any time.
//
//  * Discrete ("was it pressed on THIS frame?"): M, F11, Esc.
//    Fed from pollEvent so they fire exactly once per press, not every
//    frame while held. main() must call HandleEvent() for each polled
//    event, and BeginFrame() once per loop BEFORE polling.
class Input
{
public:
    // --- frame lifecycle ---

    // Clears this frame's discrete-press flags. Call once per frame,
    // BEFORE polling events.
    void BeginFrame();

    // Records discrete key presses. Call for every event returned by
    // RenderWindow::pollEvent.
    void HandleEvent(const sf::Event& event);

    // --- continuous (held) ---

    // Movement direction from WASD, pre-scaled by sprintMultiplier while
    // Shift is held. Normalising this (as Player does) removes the sprint
    // scale, so facing/aim are unaffected.
    sf::Vector2f GetMovement();

    // Edge-triggered: true only on the frame firing is first pressed.
    // Space or left mouse button.
    bool IsFiring() const { return m_firing; }

    // Held: true while the dice test's magnet grab is down (E or right mouse
    // button). DiceScene pulls the die to a hold point while this is true and
    // fires it along the aim when it goes false.
    bool IsHoldingDice() const;

    // Held: true while the middle mouse button is down, which orbits the
    // dice-test camera.
    bool IsCameraOrbiting() const;

    // --- discrete (true only on the frame pressed) ---

    bool WasMenuPressed() const { return m_menu; }              // M
    bool WasFullscreenPressed() const { return m_fullscreen; }  // F11
    bool WasEscapePressed() const { return m_escape; }          // Esc
    bool WasDiceTestPressed() const { return m_diceTest; }      // F
    bool WasResetPressed() const { return m_reset; }            // R

    // Movement speed multiplier applied while Shift is held.
    float sprintMultiplier = 2.0f;

private:
    // Discrete press flags, valid until the next BeginFrame().
    bool m_menu = false;
    bool m_fullscreen = false;
    bool m_escape = false;
    bool m_diceTest = false;
    bool m_reset = false;

    // Firing edge detection.
    bool m_firing = false;
    bool m_fireWasDown = false;
};
