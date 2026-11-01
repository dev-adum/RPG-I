#include "MenuScene.h"
#include "SceneManager.h"
#include "GameplayScene.h"
#include <memory>

namespace {
    // FFVII-ish palette.
    const sf::Color kBorder      = sf::Color(120, 150, 255);   // lighter blue frame
    const sf::Color kGradTopLeft = sf::Color(30, 60, 170);     // blue, top-left
    const sf::Color kGradBotRight = sf::Color(0, 0, 0);        // black, bottom-right

    constexpr float kPanelFractionX = 0.70f;   // 70% of the window
    constexpr float kPanelFractionY = 0.70f;
    constexpr float kBorderThickness = 3.f;
}

MenuScene::MenuScene(SceneManager& sceneManager, GameContext& context,
    const std::string& returnMap, sf::Vector2f returnPlayerPos)
    : m_sceneManager(sceneManager), m_context(context),
      m_returnMap(returnMap), m_returnPlayerPos(returnPlayerPos)
{
    m_fill.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
    m_fill.resize(4);

    Layout();
}

void MenuScene::Layout()
{
    const sf::Vector2f win(static_cast<float>(m_context.window.getSize().x),
                           static_cast<float>(m_context.window.getSize().y));

    m_panelSize = { win.x * kPanelFractionX, win.y * kPanelFractionY };
    m_panelPos  = { (win.x - m_panelSize.x) * 0.5f,
                    (win.y - m_panelSize.y) * 0.5f };

    // Border sits behind the fill, slightly larger all round.
    m_border.setPosition({ m_panelPos.x - kBorderThickness,
                           m_panelPos.y - kBorderThickness });
    m_border.setSize({ m_panelSize.x + kBorderThickness * 2.f,
                       m_panelSize.y + kBorderThickness * 2.f });
    m_border.setFillColor(kBorder);

    // Diagonal gradient: 4 corners interpolated across a triangle strip.
    // Order: top-left, top-right, bottom-left, bottom-right.
    const float l = m_panelPos.x;
    const float t = m_panelPos.y;
    const float r = m_panelPos.x + m_panelSize.x;
    const float b = m_panelPos.y + m_panelSize.y;

    m_fill[0] = sf::Vertex{ { l, t }, kGradTopLeft };
    m_fill[1] = sf::Vertex{ { r, t }, kGradTopLeft };
    m_fill[2] = sf::Vertex{ { l, b }, kGradBotRight };
    m_fill[3] = sf::Vertex{ { r, b }, kGradBotRight };
}

void MenuScene::HandleInput()
{
    // M or Esc closes the menu.
    if (m_context.input.WasMenuPressed() || m_context.input.WasEscapePressed())
    {
        RequestReturn();
    }
}

void MenuScene::RequestReturn()
{
    SceneManager& sm = m_sceneManager;
    GameContext& ctx = m_context;
    const std::string map = m_returnMap;
    const sf::Vector2f pos = m_returnPlayerPos;

    m_sceneManager.RequestScene([&sm, &ctx, map, pos]()
    {
        return std::make_unique<GameplayScene>(sm, ctx, map, pos);
    });
}

void MenuScene::Update(float dt)
{
    (void)dt;
    // Stateless for now - entrance is handled by SceneManager's fade.
}

void MenuScene::Render(sf::RenderWindow& window)
{
    // Draw in screen space so the panel does not scroll with the camera.
    window.setView(window.getDefaultView());

    window.draw(m_border);
    window.draw(m_fill);
}
