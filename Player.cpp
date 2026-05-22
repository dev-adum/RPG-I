#include "Player.h"


Player::Player()
    : playerSprite(playerTexture),
    reticuleSprite(reticuleTexture),
    walkSound(walkBuffer),
    VoidTrack(),
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

   


    if (!VoidTrack.openFromFile("Assets/Music/Void/VoidTrack.wav"))
    {
        std::cout << "Failed to load VoidTrack.wav\n";
    }

    VoidTrack.setLooping(true);
    VoidTrack.play();



}


void Player::Initialize()
{
    sf::Vector2f playerPos = playerSprite.getPosition();
 
  
        playerBoxSize = sf::Vector2i({ 16,32 });

        playerBox.setFillColor(sf::Color::Transparent);
        playerBox.setOutlineColor(sf::Color::Transparent);
        playerBox.setOutlineThickness(1);
        playerBox.setSize(sf::Vector2f(playerBoxSize.x, playerBoxSize.y));
        playerBox.setOrigin({ 9,10 });
   



    sf::IntRect frame_1({ 0, 0 }, { 64, 64 });
   sf::IntRect frame_2({ 64, 0 }, { 64, 64 });
   sf::IntRect frame_3({ 128, 0 }, { 64, 64 });
   sf::IntRect frame_4({ 192, 0 }, { 64, 64 });

   sf::IntRect frame_5({ 256, 0 }, { 64, 64 });
   sf::IntRect frame_6({ 320, 0 }, { 64, 64 });
   sf::IntRect frame_7({ 384, 0 }, { 64, 64 });
   sf::IntRect frame_8({ 448, 0 }, { 64, 64 });

   sf::IntRect frame_9({ 0, 64 }, { 64, 64 });
   sf::IntRect frame_10({ 64, 64 }, { 64, 64 });
   sf::IntRect frame_11({ 128, 64 }, { 64, 64 });
   sf::IntRect frame_12({ 192, 64 }, { 64, 64 });

   sf::IntRect frame_13({ 256, 64 }, { 64, 64 });
   sf::IntRect frame_14({ 320, 64 }, { 64, 64 });
   sf::IntRect frame_15({ 384, 64 }, { 64, 64 });
   sf::IntRect frame_16({ 448, 64 }, { 64, 64 });
    
   sf::IntRect frame_17({ 0, 128 }, { 64, 64 });
   sf::IntRect frame_18({ 64, 128 }, { 64, 64 });
   sf::IntRect frame_19({ 128, 128 }, { 64, 64 });
   sf::IntRect frame_20({ 192, 128 }, { 64, 64 });

   sf::IntRect frame_21({ 256, 128 }, { 64, 64 });
   sf::IntRect frame_22({ 320, 128 }, { 64, 64 });
   sf::IntRect frame_23({ 384, 128 }, { 64, 64 });
   sf::IntRect frame_24({ 448, 128 }, { 64, 64 });

   sf::IntRect frame_25({ 0, 192 }, { 64, 64 });
   sf::IntRect frame_26({ 64, 192 }, { 64, 64 });
   sf::IntRect frame_27({ 128, 192 }, { 64, 64 });
   sf::IntRect frame_28({ 192, 192 }, { 64, 64 });

   sf::IntRect frame_29({ 256, 192 }, { 64, 64 });
   sf::IntRect frame_30({ 320, 192 }, { 64, 64 });
   sf::IntRect frame_31({ 384, 192 }, { 64, 64 });
   sf::IntRect frame_32({ 448, 192 }, { 64, 64 });

    

    
    playerSprite.setOrigin({ 32.f, 32.f });
    reticuleSprite.setOrigin({ 32.f, 32.f });
    
    

	SouthWest = { frame_1, frame_2 };
	South = { frame_3, frame_4 };
	SouthEast = { frame_5, frame_6 };
	East = { frame_7, frame_8 };
	NorthEast = { frame_9, frame_10 };
	North = { frame_11, frame_12 };
	NorthWest = { frame_13, frame_14 };
	West = { frame_15, frame_16 };

    SouthWestFire = { frame_17, frame_18 };
    SouthFire = { frame_19, frame_20 };
    SouthEastFire = { frame_21, frame_22 };
    EastFire = { frame_23, frame_24 };
    NorthEastFire = { frame_25, frame_26 };
    NorthFire = { frame_27, frame_28 };
    NorthWestFire = { frame_29, frame_30 };
    WestFire = { frame_31, frame_32 };




	Reticule = { frame_1, frame_2 };




}


sf::Vector2f Player::GetClampedAim(sf::RenderWindow& window, float maxDistance)
{
    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);

    sf::Vector2f playerPos = playerSprite.getPosition();

    sf::Vector2f offset = mouseWorld - playerPos;

    float length = std::sqrt(offset.x * offset.x + offset.y * offset.y);

    if (length > maxDistance && length > 0.f)
    {
        offset /= length;
        offset *= maxDistance;
    }

    return playerPos + offset;
}






void Player::Update(sf::Vector2f dir, sf::Vector2f aimDir, float delta_time, const sf::View& view, Enemy& enemy)
{
    sf::Vector2f playerPos = playerSprite.getPosition();





    isFiring = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    bool isMoving = (dir.x != 0.f || dir.y != 0.f);




    // aimDir is already "mouse - player", so we classify direction directly
    float angle = std::atan2(aimDir.y, aimDir.x) * 180.f / 3.14159f;

    // normalize angle to 0–360
    if (angle < 0.f)
        angle += 360.f;

    // 8-direction slicing (each 45 degrees)
    if (angle >= 337.5f || angle < 22.5f)
    {
        anim = &East;
        currentIdx = &e_idx;
    }
    else if (angle < 67.5f)
    {
        anim = &SouthEast;

        currentIdx = &se_idx;
    }
    else if (angle < 112.5f)
    {
        anim = &South;
        currentIdx = &s_idx;
    }
    else if (angle < 157.5f)
    {
        anim = &SouthWest;
        currentIdx = &sw_idx;
    }
    else if (angle < 202.5f)
    {
        anim = &West;
        currentIdx = &w_idx;
    }
    else if (angle < 247.5f)
    {
        anim = &NorthWest;
        currentIdx = &nw_idx;
    }
    else if (angle < 292.5f)
    {
        anim = &North;
        currentIdx = &n_idx;
    }
    else
    {
        anim = &NorthEast;
        currentIdx = &ne_idx;
    }



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

    if (anim == &SouthWest)
        fire_anim = &SouthWestFire;
    else if (anim == &South)
        fire_anim = &SouthFire;
    else if (anim == &SouthEast)
        fire_anim = &SouthEastFire;
    else if (anim == &East)
        fire_anim = &EastFire;
    else if (anim == &NorthEast)
        fire_anim = &NorthEastFire;
    else if (anim == &North)
        fire_anim = &NorthFire;
    else if (anim == &NorthWest)
        fire_anim = &NorthWestFire;
    else if (anim == &West)
        fire_anim = &WestFire;








    player_anim_timer += delta_time;

    if (isMoving)
    {

        if (player_anim_timer >= player_anim_speed)
        {
            player_anim_timer = 0.f;
            *currentIdx = (*currentIdx + 1) % anim->size();
        }

    }
    else
    {
        *currentIdx = 0;
        player_anim_timer = 0.f;
    }

    reticule_anim_timer += delta_time;

    if (reticule_anim_timer >= reticule_anim_speed)
    {
        reticule_anim_timer = 0.f;
        targ_idx = (targ_idx + 1) % Reticule.size();
    }

    reticuleSprite.setTextureRect((Reticule[targ_idx]));

    const std::vector<sf::IntRect>* finalAnim = anim;
    unsigned int* finalIdx = currentIdx;

    if (isFiring && fire_anim != nullptr && fire_anim->size() >= 2)
    {
        finalAnim = fire_anim;

        if (anim == &SouthWest) finalIdx = &sw_fire_idx;
        else if (anim == &South) finalIdx = &s_fire_idx;
        else if (anim == &SouthEast) finalIdx = &se_fire_idx;
        else if (anim == &East) finalIdx = &e_fire_idx;
        else if (anim == &NorthEast) finalIdx = &ne_fire_idx;
        else if (anim == &North) finalIdx = &n_fire_idx;
        else if (anim == &NorthWest) finalIdx = &nw_fire_idx;
        else if (anim == &West) finalIdx = &w_fire_idx;
    }

    if (isFiring && finalAnim && finalAnim->size() >= 2)
    {
        fire_anim_timer += delta_time;

        if (fire_anim_timer >= fire_anim_speed)
        {
            fire_anim_timer = 0.f;

            // advance fire animation
            (*finalIdx)++;

            // clamp / loop only over fire frames
            if (*finalIdx >= finalAnim->size())
                *finalIdx = 0;
        }
    }
    else
    {
        fire_anim_timer = 0.f;

        // IMPORTANT: only reset fire indices (not movement indices)
        sw_fire_idx = s_fire_idx = se_fire_idx =
            e_fire_idx = ne_fire_idx = n_fire_idx =
            nw_fire_idx = w_fire_idx = 0;
    }

    if (finalAnim && *finalIdx < finalAnim->size())
        playerSprite.setTextureRect((*finalAnim)[*finalIdx]);

    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    float minX = center.x - size.x / 2.f;
    float maxX = center.x + size.x / 2.f;
    float minY = center.y - size.y / 2.f;
    float maxY = center.y + size.y / 2.f;


    // define body (feet area)
    float bodyW = 16.f;
    float bodyH = 16.f;

    // shift it DOWN inside the 64x64 frame
    float offsetY = 10.f / 2.f - bodyH / 2.f;

    float bodyLeft = playerPos.x - bodyW / 2.f;
    float bodyTop = playerPos.y + offsetY - bodyH / 2.f;



    // left
    if (bodyLeft < minX)
        playerPos.x += (minX - bodyLeft);

    // right
    if (bodyLeft + bodyW > maxX)
        playerPos.x -= (bodyLeft + bodyW - maxX);

    // top
    if (bodyTop < minY)
        playerPos.y += (minY - bodyTop);

    // bottom
    if (bodyTop + bodyH > maxY)
        playerPos.y -= (bodyTop + bodyH - maxY);









    sf::Vector2f currentPos = playerSprite.getPosition();
    sf::Vector2f move = dir * moveSpeed * delta_time;

    // move hitbox first
    playerBox.move(move);

    // collision (box vs box)
    if (Math::DetectCollision(
        playerBox.getGlobalBounds(),
        enemy.enemyBox.getGlobalBounds()))
    {
        // undo move
        playerBox.move(-move);
    }

    // sync visuals AFTER physics
    sf::Vector2f pos = playerBox.getPosition();
    playerSprite.setPosition(pos);
   
}

void Player::Draw(sf::RenderWindow& window)
{
    window.draw(playerSprite);
    window.draw(reticuleSprite);
    window.draw(playerBox);

}