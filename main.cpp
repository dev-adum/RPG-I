#include <SFML/Graphics.hpp>


   

int main()
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    
    sf::RenderWindow window(sf::VideoMode({ 300, 300 }), "Level I");
    sf::Texture playerTexture;
    
    if (!playerTexture.loadFromFile("Assets/Player/Textures/Player_Walk.png"));
       
    

    sf::Sprite playerSprite(playerTexture);
    
    sf::IntRect frame_1({ 0, 0 }, { 64, 64 });
    sf::IntRect frame_2({ 64, 0 }, { 64, 64 });
    sf::IntRect frame_3({ 128, 0 }, { 64, 64 });
    sf::IntRect frame_4({ 192, 0 }, { 64, 64 });
  
    std::vector<sf::IntRect> SouthWest = { frame_1, frame_2 };
    std::vector<sf::IntRect> South = { frame_3, frame_4 };
    

  

    unsigned int sw_idx = 0;
    unsigned int s_idx = 0;
    float anim_timer = 0.0f;
    float const anim_speed = 0.35f;

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

       

        anim_timer += delta_time;

        if (anim_timer >= anim_speed)
        {
            anim_timer = 0.f;
            *currentIdx = (*currentIdx + 1) % anim->size();
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
        playerSprite.move(dir * 0.01f);
      
      
        if (dir.y > 0 && dir.x == 0)
        {
            anim = &South;
            currentIdx = &s_idx;
        }
        else if (dir.y > 0 && dir.x < 0)
        {
            anim = &SouthWest;
            currentIdx = &sw_idx;
        }
     
        playerSprite.setTextureRect((*anim)[*currentIdx]);

        window.clear();
        window.draw(playerSprite);
        window.display();
    }
}