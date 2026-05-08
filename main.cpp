#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <ctime>

sf::Vector2f NormalizeVector(sf::Vector2f vector)
{
    float m = std::sqrt(vector.x * vector.x + vector.y * vector.y);

    sf::Vector2f normalizedVector;

    normalizedVector.x = vector.x / m;
    normalizedVector.y = vector.y / m;

    return normalizedVector;
}

     

int main()
{ 
    srand(time(nullptr));

    const sf::Vector2f view_size = { 300.f, 300.f };


    sf::RenderWindow window(sf::VideoMode({ 300, 300 }), "Level I");
    
    sf::View view(sf::FloatRect({ 0.f, 0.f }, { 300.f, 300.f }));
    window.setView(view);
    
    
    sf::Vector2u windowSize = window.getSize();
    

    sf::Texture playerTexture;
    sf::Sprite playerSprite(playerTexture);
    sf::Vector2u playerSize = playerTexture.getSize();

    struct FadingSound
    {
        sf::Sound sound;
        float fadeSpeed = 250.f;

        bool active = false;
        float playTime = 0.f;
        float maxDuration = 1.f;

        void play()
        {
            sound.setVolume(100.f);
            sound.play();

            active = true;
            playTime = 0.f;
        }

        void update(float dt)
        {
            if (!active) return;

            playTime += dt;

            // HARD CUT at 1 second
            if (playTime >= maxDuration)
            {
                sound.stop();
                sound.setVolume(100.f);
                active = false;
                return;
            }

            // optional fade-out near end
            float remaining = maxDuration - playTime;

            if (remaining < 0.2f)
            {
                float v = sound.getVolume();
                v -= fadeSpeed * dt;

                if (v <= 0.f)
                {
                    sound.stop();
                    active = false;
                }
                else
                {
                    sound.setVolume(v);
                }
            }
        }
    };



    sf::Texture targetTexture;
    sf::Sprite targetSprite(targetTexture);

    struct Bullet
    {
        sf::RectangleShape shape;
        sf::Vector2f direction;
        float life = 3.f;
    };

    std::vector<Bullet> bullets;



    float bulletSpeed = 0.075f;

  
    float fireCooldown = 0.25f;   
    float fireTimer = 0.f;
    bool canFire = true;
    
   
    if (!targetTexture.loadFromFile("Assets/Player/Textures/Target_Point.png"))
    {
        return EXIT_FAILURE;
    }

    
    if (!playerTexture.loadFromFile("Assets/Player/Textures/Player_Walk.png"))
    {
        return EXIT_FAILURE;
    }
   
    sf::SoundBuffer walkBuffer;
    if (!walkBuffer.loadFromFile("Assets/Player/SFX/Player_Step.wav"))
    {
        return EXIT_FAILURE;
    }

    sf::SoundBuffer fireBuffer1;
    if (!fireBuffer1.loadFromFile("Assets/Player/SFX/Bang_1.wav"))
    {
        return EXIT_FAILURE;
    }

    sf::SoundBuffer fireBuffer2;
    if (!fireBuffer2.loadFromFile("Assets/Player/SFX/Bang_2.wav"))
    {
        return EXIT_FAILURE;
    }

    sf::Sound fireSound1(fireBuffer1);
    fireSound1.setVolume(5);

    sf::Sound fireSound2(fireBuffer2);
    fireSound2.setVolume(5);

   

    sf::Sound walkSound(walkBuffer);
 
    walkSound.setLooping(true);
   
    sf::Music VoidTrack;

    if (!VoidTrack.openFromFile("Assets/Music/Void/VoidTrack.wav"))
    {
        return EXIT_FAILURE;
    }

   



    VoidTrack.setLooping(true);
    VoidTrack.play();
    
    // audio ^

    sf::Font font;
    if (!font.openFromFile("Assets/Fonts/Helvetica.ttf"))
    {
       return EXIT_FAILURE;
   }

    playerSprite.setOrigin({
        playerSize.x / 2.f + 29.f,
        playerSize.y / 2.f + 30.f
        });

    playerSprite.setPosition({
        windowSize.x / 2.f,
        windowSize.y / 2.f
        });

    targetSprite.setOrigin({
        playerSize.x / 2.f + 33.f,
        playerSize.y / 2.f + 30.f
        });

    targetSprite.setPosition({150.f, 100.f});


    
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

  
    std::vector<sf::IntRect> SouthWest = { frame_1, frame_2 };
    std::vector<sf::IntRect> South = { frame_3, frame_4 };
    std::vector<sf::IntRect> SouthEast = { frame_5, frame_6 };
    std::vector<sf::IntRect> East = { frame_7, frame_8 };
    std::vector<sf::IntRect> NorthEast = { frame_9, frame_10 };
    std::vector<sf::IntRect> North = { frame_11, frame_12 };
    std::vector<sf::IntRect> NorthWest = { frame_13, frame_14 };
    std::vector<sf::IntRect> West = { frame_15, frame_16 };

    std::vector<sf::IntRect> Target = { frame_1, frame_2 };

    unsigned int sw_idx = 0;
    unsigned int s_idx = 0;
    unsigned int se_idx = 0;
    unsigned int e_idx = 0;
    unsigned int ne_idx = 0;
    unsigned int n_idx = 0;
    unsigned int nw_idx = 0;
    unsigned int w_idx = 0;
    
    unsigned int targ_idx = 0;
    
    float player_anim_timer = 0.0f;
    float const player_anim_speed = 0.15f;

    float target_anim_timer = 0.0f;
    float const target_anim_speed = 0.8f;


    sf::Clock clock;
    sf::Vector2f dir(0.f, 0.f);


    const std::vector<sf::IntRect>* anim = &South;
    unsigned int* currentIdx = &s_idx;

    
   

    //sf::Vector2f bulletDirection = targetSprite.getPosition() - bullet.getPosition();
    //bulletDirection = NormalizeVector(bulletDirection);

    sf::Text text(font);
    text.setString("Move: Arrows/WASD  \n Fire: LMB or 'F'");
    text.setCharacterSize(7);
    text.setPosition({ 30,30 });

    while (window.isOpen())
    {
        dir = sf::Vector2f(0.f, 0.f);
        
        float const delta_time = clock.restart().asSeconds();

      
        //scaling
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::Resized>())
            {
                auto size = event->getIf<sf::Event::Resized>()->size;

                sf::View view(sf::FloatRect({ 0.f, 0.f }, { 300.f, 300.f }));

                float windowAspect = float(size.x) / float(size.y);
                float worldAspect = 1.f;

                if (windowAspect > worldAspect)
                    view.setSize({ 300.f * windowAspect, 300.f });
                else
                    view.setSize({ 300.f, 300.f / windowAspect });

                view.setCenter({ 150.f, 150.f });
                window.setView(view);
            }
        }

        
        //bullet.setPosition(bullet.getPosition() + bulletDirection * bulletSpeed);

        sf::Vector2f position = playerSprite.getPosition();
      

            if
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            {
                dir.y -= 1.f;
            }

            if
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            {
                dir.y += 1.f;
            }

            if
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            {
                dir.x += 1.f;
            }

            if
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            {
                dir.x -= 1.f;
            }
        
            fireTimer += delta_time;

            bool firePressed =
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F) ||
                sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

            if (fireTimer >= fireCooldown && firePressed)
            {
                fireTimer = 0.f;

                Bullet b;
                b.shape = sf::RectangleShape(sf::Vector2f(2.f, 2.f));
                b.shape.setPosition(playerSprite.getPosition());

                b.direction = NormalizeVector(
                    targetSprite.getPosition() - b.shape.getPosition()
                );

                bullets.push_back(b);

                if (std::rand() % 2 == 0)
                {
                    fireSound1.play();
                }
                else
                {
                    fireSound2.play();
                }bullets.push_back(b);
           
            
            
            }
      
           


            bool isMoving = (dir.x != 0.f || dir.y != 0.f);
            
            if (isMoving)
            {
                 player_anim_timer += delta_time;

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
            
            target_anim_timer += delta_time;

            if (target_anim_timer >= target_anim_speed)
            {
                target_anim_timer = 0.f;
                targ_idx = (targ_idx + 1) % Target.size();
            }

            
        playerSprite.move(dir * 0.075f);
      
      
        if (dir.y > 0 && dir.x == 0)
        {
            anim = &South;
            currentIdx = &s_idx;
        }
        
        if (dir.y > 0 && dir.x > 0)
        {
            anim = &SouthEast;
            currentIdx = &se_idx;
        }
            

        if (dir.y == 0 && dir.x > 0)
        {
            anim = &East;
            currentIdx = &e_idx;
        }

        if (dir.y < 0 && dir.x > 0)
        {
            anim = &NorthEast;
            currentIdx = &ne_idx;
        }

        if (dir.y < 0 && dir.x == 0)
        {
            anim = &North;
            currentIdx = &n_idx;
        }

        if (dir.y < 0 && dir.x < 0)
        {
            anim = &NorthWest;
            currentIdx = &nw_idx;
        }

        if (dir.y == 0 && dir.x < 0)
        {
            anim = &West;
            currentIdx = &w_idx;
        }

        if (dir.y > 0 && dir.x < 0)
        {
            
            anim = &SouthWest;
            currentIdx = &sw_idx;
        
        }
     
        target_anim_timer += delta_time;

if (target_anim_timer >= target_anim_speed)
{
    target_anim_timer = 0.f;
    targ_idx = (targ_idx + 1) % Target.size();
}

      
        sf::Vector2f pos = playerSprite.getPosition();

        // define body (feet area)
        float bodyW = 8.f;
        float bodyH = 16.f;

        // shift it DOWN inside the 64x64 frame
        float offsetY = 64.f / 2.f - bodyH / 2.f;

        float bodyLeft = pos.x - bodyW / 2.f;
        float bodyTop = pos.y + offsetY - bodyH / 2.f;

        float minX = 0.f;
        float minY = 0.f;
        float maxX = 300.f;
        float maxY = 300.f;

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


        playerSprite.setTextureRect((*anim)[*currentIdx]);
        targetSprite.setTextureRect((Target[targ_idx]));

      

        for (auto& b : bullets)
        {
            b.shape.move(b.direction * bulletSpeed);
            b.life -= delta_time;
        }

        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet& b) { return b.life <= 0.f; }),
            bullets.end()
        );

        window.clear();
        window.draw(playerSprite);
        window.draw(targetSprite);
        window.draw(text);
        
        for (auto& b : bullets)
        {
            window.draw(b.shape);
        }
     
        window.display();
        window.setView(window.getView());

        }
}