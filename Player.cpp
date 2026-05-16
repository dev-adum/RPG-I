#include "Player.h"


Player::Player()
    : playerSprite(playerTexture),
    targetSprite(targetTexture),
    fireSound1(fireBuffer1),
    fireSound2(fireBuffer2),
    walkSound(walkBuffer),
    VoidTrack()
{

}


void Player::Load()
{
    
    if (!playerTexture.loadFromFile("Assets/Player/Textures/Player_Walk.png"))
    {
        std::cout << "Failed to load Player_Walk.png\n";
    }

    if (!targetTexture.loadFromFile("Assets/Player/Textures/Target_Point.png"))
    {
        std::cout << "Failed to load Target_Point.png\n";
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

    fireSound1.setBuffer(fireBuffer1);
    fireSound1.setVolume(5);

    if (!fireBuffer2.loadFromFile("Assets/Player/SFX/Bang_2.wav"))
    {
        std::cout << "Failed to load Bang_2.wav\n";
    }

    fireSound2.setBuffer(fireBuffer2);
    fireSound2.setVolume(5);

    if (!VoidTrack.openFromFile("Assets/Music/Void/VoidTrack.wav"))
    {
        std::cout << "Failed to load VoidTrack.wav\n";
    }

    VoidTrack.setLooping(true);
    VoidTrack.play();



}


void Player::Initialize()
{
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
    
    
    sf::Vector2f position = playerSprite.getPosition();
    
    sf::Vector2f pos = playerSprite.getPosition();

    
    playerSprite.setOrigin({ 32.f, 32.f });
    targetSprite.setOrigin({ 32.f, 32.f });
    
    

	SouthWest = { frame_1, frame_2 };
	South = { frame_3, frame_4 };
	SouthEast = { frame_5, frame_6 };
	East = { frame_7, frame_8 };
	NorthEast = { frame_9, frame_10 };
	North = { frame_11, frame_12 };
	NorthWest = { frame_13, frame_14 };
	West = { frame_15, frame_16 };

	Target = { frame_1, frame_2 };



    
    // define body (feet area)
    float bodyW = 8.f;
    float bodyH = 16.f;

    // shift it DOWN inside the 64x64 frame
    float offsetY = 64.f / 2.f - bodyH / 2.f;

    float bodyLeft = pos.x - bodyW / 2.f;
    float bodyTop = pos.y + offsetY - bodyH / 2.f;

    float minX = 0.f;
    float minY = 0.f;
    float maxX = 150.f;
    float maxY = 150.f;

    // left
    if (bodyLeft < minX)
        pos.x += (minX - bodyLeft);

    // right
    if (bodyLeft + bodyW > maxX)
        pos.x -= (bodyLeft + bodyW - maxX);

    // top
    if (bodyTop < minY)
        pos.y += (minY - bodyTop);

    // bottom
    if (bodyTop + bodyH > maxY)
        pos.y -= (bodyTop + bodyH - maxY);

    playerSprite.setPosition(pos);
    

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








void Player::Update(sf::Vector2f dir, sf::Vector2f aimDir, float delta_time)
{
   
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
    
    bool isMoving = (dir.x != 0.f || dir.y != 0.f);
    playerSprite.move(dir * 0.075f);


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

    target_anim_timer += delta_time;
    
    if (target_anim_timer >= target_anim_speed)
    {
        target_anim_timer = 0.f;
        targ_idx = (targ_idx + 1) % Target.size();
    }
    
    playerSprite.setTextureRect((*anim)[*currentIdx]);
    targetSprite.setTextureRect((Target[targ_idx]));


}
