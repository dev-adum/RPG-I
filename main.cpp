#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

int main()
{
    
    sf::RenderWindow window(sf::VideoMode({ 300, 300 }), "Level I");
    
    sf::Texture playerTexture;
    sf::Sprite playerSprite(playerTexture);

    sf::Vector2u windowSize = window.getSize();
    sf::Vector2u texSize = playerTexture.getSize();
    
    window.setView(sf::View(sf::FloatRect({ 0.f, 0.f }, { 300.f, 300.f })));
   
    
    if (!playerTexture.loadFromFile("Assets/Player/Textures/Player_Walk.png"))
    {
        return EXIT_FAILURE;
    }
   
    sf::SoundBuffer walkBuffer;
    if (!walkBuffer.loadFromFile("Assets/Player/SFX/Player_Step.wav"))
    {
        return EXIT_FAILURE;
    }

    sf::Sound walkSound(walkBuffer);
 
    walkSound.setLooping(true);
   
    sf::Music VoidTrack;

    if (!VoidTrack.openFromFile("Assets/Music/Void/VoidTrack.wav"))
    {
        return EXIT_FAILURE;
    }

    VoidTrack.setLooping(true);
    VoidTrack.play();
    

    playerSprite.setOrigin({
        texSize.x / 2.f,
        texSize.y / 2.f
        });

    playerSprite.setPosition({
        windowSize.x / 2.f,
        windowSize.y / 2.f
        });


    
    sf::IntRect frame_1({ 0, 0 }, { 64, 64 });
    sf::IntRect frame_2({ 64, 0 }, { 64, 64 });
    sf::IntRect frame_3({ 128, 0 }, { 64, 64 });
    sf::IntRect frame_4({ 192, 0 }, { 64, 64 });

    sf::IntRect frame_5({ 256, 0 }, { 64, 64 });
    sf::IntRect frame_6({ 320, 0 }, { 64, 64 });
    sf::IntRect frame_7({ 386, 0 }, { 64, 64 });
    sf::IntRect frame_8({ 450, 0 }, { 64, 64 });

    sf::IntRect frame_9({ 0, 64 }, { 64, 64 });
    sf::IntRect frame_10({ 64, 64 }, { 64, 64 });
    sf::IntRect frame_11({ 128, 64 }, { 64, 64 });
    sf::IntRect frame_12({ 192, 64 }, { 64, 64 });

    sf::IntRect frame_13({ 256, 64 }, { 64, 64 });
    sf::IntRect frame_14({ 320, 64 }, { 64, 64 });
    sf::IntRect frame_15({ 386, 64 }, { 64, 64 });
    sf::IntRect frame_16({ 450, 64 }, { 64, 64 });

  
    std::vector<sf::IntRect> SouthWest = { frame_1, frame_2 };
    std::vector<sf::IntRect> South = { frame_3, frame_4 };
    std::vector<sf::IntRect> SouthEast = { frame_5, frame_6 };
    std::vector<sf::IntRect> East = { frame_7, frame_8 };
    std::vector<sf::IntRect> NorthEast = { frame_9, frame_10 };
    std::vector<sf::IntRect> North = { frame_11, frame_12 };
    std::vector<sf::IntRect> NorthWest = { frame_13, frame_14 };
    std::vector<sf::IntRect> West = { frame_15, frame_16 };

    unsigned int sw_idx = 0;
    unsigned int s_idx = 0;
    unsigned int se_idx = 0;
    unsigned int e_idx = 0;
    unsigned int ne_idx = 0;
    unsigned int n_idx = 0;
    unsigned int nw_idx = 0;
    unsigned int w_idx = 0;
    float anim_timer = 0.0f;
    float const anim_speed = 0.2f;

    sf::Clock clock;
    sf::Vector2f dir(0.f, 0.f);


    const std::vector<sf::IntRect>* anim = &SouthWest;
    unsigned int* currentIdx = &sw_idx;

  

    while (window.isOpen())
    {
        dir = sf::Vector2f(0.f, 0.f);
        
        float const delta_time = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }


      

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
        
      
            bool isMoving = (dir.x != 0.f || dir.y != 0.f);

            if (isMoving)
            {
                 anim_timer += delta_time;

                if (anim_timer >= anim_speed)
                {
                    anim_timer = 0.f;
                    *currentIdx = (*currentIdx + 1) % anim->size();
                }

            }
            else
            {
                *currentIdx = 0;
                anim_timer = 0.f;
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
            
            
            
        playerSprite.move(dir * 0.01f);
      
      
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
     




        playerSprite.setTextureRect((*anim)[*currentIdx]);

        window.clear();
        window.draw(playerSprite);
        window.display();
        window.setView(window.getView());

        }
}