#include "Player.h"

Player::Player()
    : playerSprite(playerTexture),
    reticuleSprite(reticuleTexture),
    walkSound(walkBuffer),
    fireSound1(fireBuffer1),
    fireSound2(fireBuffer2)
{
}

void Player::Load()
{
    if (!playerTexture.loadFromFile("Assets/Player/Textures/Player_Walk.png"))
    {
        std::cout << "Failed to load Player_Walk.png\n";
    }

    if (!reticuleTexture.loadFromFile("Assets/Player/Textures/Reticule_Mark.png"))
    {
        std::cout << "Failed to load Reticule_Mark.png\n";
    }

    if (!walkBuffer.loadFromFile("Assets/Player/SFX/Player_Step.wav"))
    {
        std::cout << "Failed to load Player_Step.wav\n";
    }

    walkSound.setBuffer(walkBuffer);
    walkSound.setLooping(true);

    if (!fireBuffer1.loadFromFile("Assets/Player/SFX/Bang_1.wav"))
    {
        std::cout << "Failed to load Bang_1.wav\n";
    }

    if (!fireBuffer2.loadFromFile("Assets/Player/SFX/Bang_2.wav"))
    {
        std::cout << "Failed to load Bang_2.wav\n";
    }
}

void Player::Initialize()
{
    fireSound1.setVolume(10.f);
    fireSound2.setVolume(10.f);

    playerBoxSize = sf::Vector2i({ 10,25 });

    playerBox.setFillColor(sf::Color::Transparent);
    playerBox.setOutlineColor(sf::Color::Transparent);
    playerBox.setOutlineThickness(1);
    playerBox.setSize(sf::Vector2f(playerBoxSize.x, playerBoxSize.y));
    playerBox.setOrigin({ 9,10 });

    playerSprite.setOrigin({ 32.f, 32.f });
    reticuleSprite.setOrigin({ 32.f, 32.f });

    // --- AUTOMATED SPRITESHEET SLICING (Walk Animations) ---
    // Row 0 (Y = 0)
    animManager.AddRow("Walk_East", 384, 0, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_SouthEast", 256, 0, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_South", 128, 0, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_SouthWest", 0, 0, 64, 64, 2, 0.1f);

    // Row 1 (Y = 64)
    animManager.AddRow("Walk_West", 384, 64, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_NorthWest", 256, 64, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_North", 128, 64, 64, 64, 2, 0.1f);
    animManager.AddRow("Walk_NorthEast", 0, 64, 64, 64, 2, 0.1f);

    // --- AUTOMATED SPRITESHEET SLICING (Fire Animations) ---
    // Row 2 (Y = 128)
    animManager.AddRow("Fire_East", 384, 128, 64, 64, 2, 0.05f);
    animManager.AddRow("Fire_SouthEast", 256, 128, 64, 64, 2, 0.05f);
    animManager.AddRow("Fire_South", 128, 128, 64, 64, 2, 0.05f);
    animManager.AddRow("Fire_SouthWest", 0, 128, 64, 64, 2, 0.05f);

    // Row 3 (Y = 192)
    animManager.AddRow("Fire_West", 384, 192, 64, 64, 2, 0.05f);
    animManager.AddRow("Fire_NorthWest", 256, 192, 64, 64, 2, 0.05f);
    animManager.AddRow("Fire_North", 128, 192, 64, 64, 2, 0.05f);
    animManager.AddRow("Fire_NorthEast", 0, 192, 64, 64, 2, 0.05f);

    // Reticule frames (using first row frame setup just for reference)
    sf::IntRect r1({ 0, 0 }, { 64, 64 });
    sf::IntRect r2({ 64, 0 }, { 64, 64 });
    Reticule = { r1, r2 };
}

sf::Vector2f Player::GetSmoothedAim(sf::RenderWindow& window, const sf::View& view, float delta_time)
{
    sf::Vector2f pos = playerBox.getPosition();
    sf::Vector2i currentMouse = sf::Mouse::getPosition(window);

    if (firstMouseCheck)
    {
        lastMousePos = currentMouse;
        firstMouseCheck = false;
    }

    sf::Vector2i mouseDelta = currentMouse - lastMousePos;
    lastMousePos = currentMouse;

    sf::Vector2f worldDelta(
        static_cast<float>(mouseDelta.x) * (view.getSize().x / static_cast<float>(window.getSize().x)),
        static_cast<float>(mouseDelta.y) * (view.getSize().y / static_cast<float>(window.getSize().y))
    );

    aimOffset += worldDelta;

    float length = std::sqrt(aimOffset.x * aimOffset.x + aimOffset.y * aimOffset.y);
    if (length > maxAimDistance)
    {
        aimOffset *= (maxAimDistance / length);
    }
    else if (length == 0.f && aimOffset.x == 0.f && aimOffset.y == 0.f)
    {
        aimOffset = { maxAimDistance, 0.f };
    }

    if (!isReticuleInitialized)
    {
        smoothedAimOffset = aimOffset;
        isReticuleInitialized = true;
    }

    float aimSmoothSpeed = 5.f;
    smoothedAimOffset.x += (aimOffset.x - smoothedAimOffset.x) * aimSmoothSpeed * delta_time;
    smoothedAimOffset.y += (aimOffset.y - smoothedAimOffset.y) * aimSmoothSpeed * delta_time;

    return pos + smoothedAimOffset;
}

void Player::Update(sf::Vector2f dir, sf::Vector2f aimDir, float delta_time,
    const sf::View& view, Enemy& enemy, BulletSystem& bulletSystem, const MapData& mapData)
{
    sf::Vector2f pos = playerBox.getPosition();
    isFiring = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    bool isMoving = (dir.x != 0.f || dir.y != 0.f);

    // --- CLEAN ANIMATION MAPPING VIA MATH ---
    std::string directionName = Math::Get8WayDirection(aimDir);
    std::string animPrefix = isFiring ? "Fire_" : "Walk_";
    std::string fullAnimName = animPrefix + directionName;

    animManager.Play(fullAnimName);

    if (isMoving || isFiring)
    {
        animManager.Update(delta_time, playerSprite);
    }
    else
    {
        animManager.frameIndex = 0;
        animManager.timer = 0.f;
        // Keep the first frame of the current direction active when stationary
        if (animManager.animations.find(fullAnimName) != animManager.animations.end() &&
            !animManager.animations[fullAnimName].frames.empty())
        {
            playerSprite.setTextureRect(animManager.animations[fullAnimName].frames[0]);
        }
    }

    // --- RETICULE ANIMATION ---
    reticule_anim_timer += delta_time;
    if (reticule_anim_timer >= reticule_anim_speed)
    {
        reticule_anim_timer = 0.f;
        targ_idx = (targ_idx + 1) % Reticule.size();
    }
    reticuleSprite.setTextureRect(Reticule[targ_idx]);

    // --- AUDIO HANDLING ---
    if (isMoving)
    {
        if (walkSound.getStatus() != sf::Sound::Status::Playing)
            walkSound.play();
        walkSound.setVolume(100.f);
    }
    else
    {
        float v = walkSound.getVolume();
        if (v > 0.f)
        {
            walkSound.setVolume(std::max(0.f, v - 850.f * delta_time));
        }
        else
        {
            walkSound.stop();
        }
    }

    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    float minX = center.x - size.x / 2.f;
    float maxX = center.x + size.x / 2.f;
    float minY = center.y - size.y / 2.f;
    float maxY = center.y + size.y / 2.f;

    float bodyW = 16.f;
    float bodyH = 16.f;
    float offsetY = 10.f / 2.f - bodyH / 2.f;

    sf::Vector2f move = dir * moveSpeed * delta_time;

    // Move hitbox first
    playerBox.move(move);

    // 1. Enemy collision
    if (Math::DetectCollision(
        playerBox.getGlobalBounds(),
        enemy.enemyBox.getGlobalBounds()))
    {
        playerBox.move(-move);
    }

    // 2. Tile collision check
    if (mapData.GetTileSizeX() > 0 && mapData.GetTileSizeY() > 0)
    {
        sf::FloatRect bounds = playerBox.getGlobalBounds();

        sf::Vector2f corners[4] = {
            { bounds.position.x, bounds.position.y },
            { bounds.position.x + bounds.size.x, bounds.position.y },
            { bounds.position.x, bounds.position.y + bounds.size.y },
            { bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y }
        };

        const auto& layout = mapData.GetData();
        bool tileBlocked = false;

        for (const auto& corner : corners)
        {
            int gridX = static_cast<int>(corner.x / mapData.GetTileSizeX());
            int gridY = static_cast<int>(corner.y / mapData.GetTileSizeY());

            if (gridY >= 0 && gridY < static_cast<int>(layout.size()) &&
                gridX >= 0 && gridX < static_cast<int>(layout[gridY].size()))
            {
                int tileId = layout[gridY][gridX];
                if (mapData.IsTileSolid(tileId))
                {
                    tileBlocked = true;
                    break;
                }
            }
            else
            {
                tileBlocked = true;
                break;
            }
        }

        if (tileBlocked)
        {
            playerBox.move(-move);
        }
    }

    pos = playerBox.getPosition();

    float bodyLeft = pos.x - bodyW / 2.f;
    float bodyTop = pos.y + offsetY - bodyH / 2.f;

    if (bodyLeft < minX)
        pos.x += (minX - bodyLeft);

    if (bodyLeft + bodyW > maxX)
        pos.x -= (bodyLeft + bodyW - maxX);

    if (bodyTop < minY)
        pos.y += (minY - bodyTop);

    if (bodyTop + bodyH > maxY)
        pos.y -= (bodyTop + bodyH - maxY);

    if (isFiring)
    {
        bool fired = bulletSystem.Shoot(playerBox.getPosition(), aimDir);

        if (fired)
        {
            sf::Sound* s = (rand() % 2 == 0)
                ? &fireSound1
                : &fireSound2;

            s->setVolume(bulletVolume);
            s->play();
        }
    }

    playerBox.setPosition(pos);
    playerSprite.setPosition(pos);
}

void Player::Draw(sf::RenderWindow& window)
{
    window.draw(playerSprite);
    window.draw(reticuleSprite);
    window.draw(playerBox);
}