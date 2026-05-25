#include "Enemy.h"


Enemy::Enemy()
    : 
    enemySprite(idleTexture),
    hitSound1(hitBuffer1),
    bubbleSprite(stopTexture)
{

}


void Enemy::Load()
{

    if (!idleTexture.loadFromFile("Assets/Enemy/Textures/Enemy_Idle.png"))
    {
        std::cout << "Failed to load Enemy_Idle.png\n";
    }

    if (!hitTexture.loadFromFile("Assets/Enemy/Textures/Enemy_Hit.png"))
    {
        std::cout << "Failed to load Enemy_Idle.png\n";
    }

    if (!hitBuffer1.loadFromFile("Assets/Enemy/SFX/Enemy_Hit.wav"))
    {
        std::cout << "Failed to load Bang_2.wav\n";
    }
}


void Enemy::Initialize()
{
    sf::Vector2f enemyPos = enemySprite.getPosition();
    

  
        enemyBoxSize = sf::Vector2i({ 16,32 });

        enemyBox.setFillColor(sf::Color::Transparent);
        enemyBox.setOutlineColor(sf::Color::Transparent);
        enemyBox.setOutlineThickness(1);
        enemyBox.setSize(sf::Vector2f(enemyBoxSize.x, enemyBoxSize.y));
        enemyBox.setOrigin({ 38,40 });

        enemyHealthBarBack.setSize({ 40.f, 5.f });
        enemyHealthBarBack.setFillColor(sf::Color::Red);

        enemyHealthBarFront.setSize({ 40.f, 5.f });
        enemyHealthBarFront.setFillColor(sf::Color::Green);


    sf::IntRect frame_1({ 0, 0 }, { 64, 64 });
    sf::IntRect frame_2({ 64, 0 }, { 64, 64 });
    sf::IntRect frame_3({ 128, 0 }, { 64, 64 });
    sf::IntRect frame_4({ 192, 0 }, { 64, 64 });


    sf::Vector2f position = enemySprite.getPosition();



    enemySprite.setOrigin({ 60.f, 60.f });

    Idle = { frame_1, frame_2, frame_3, frame_4 };
    Hit = { frame_1, frame_2, frame_3, frame_4 };

    enemySprite.setPosition({ 300.f, 300.f });

    hitSound1.setVolume(10.f);

}



void Enemy::Update(float delta_time)
{
    
    enemyHealthBarBack.setPosition({
        enemySprite.getPosition().x - 20.f,
        enemySprite.getPosition().y - 50.f
        });

    enemyHealthBarFront.setPosition({
        enemySprite.getPosition().x - 20.f,
        enemySprite.getPosition().y - 50.f
        });





    float healthPercent =
        static_cast<float>(health) / maxHealth;

    enemyHealthBarFront.setSize(
        {
            40.f * healthPercent,
            5.f
        });

    if (isHit)
    {
        enemy_hit_timer += delta_time;

        // animate frames
        enemy_state_timer += delta_time;

        if (enemy_state_timer >= enemy_hit_speed)
        {
            enemy_state_timer = 0.f;

            ht_idx++;

            if (ht_idx >= Hit.size())
                ht_idx = Hit.size() - 1; // or loop if you want looping hit anim
        }

        enemySprite.setTexture(hitTexture);
        enemySprite.setTextureRect(Hit[ht_idx]);

        // exit state AFTER fixed duration
        if (enemy_hit_timer >= hit_duration)
        {
            isHit = false;
            enemy_hit_timer = 0.f;
            enemy_state_timer = 0.f;
            ht_idx = 0;
        }
    }
    else
    {
        enemy_idle_timer += delta_time;

        if (enemy_idle_timer >= enemy_idle_speed)
        {
            enemy_idle_timer = 0.f;
            id_idx = (id_idx + 1) % Idle.size();
        }

        enemySprite.setTexture(idleTexture);
        enemySprite.setTextureRect(Idle[id_idx]);
    }

    enemyBox.setPosition(enemySprite.getPosition());
}

void Enemy::TakeDamage(int damage)
{
    health -= damage;

    isHit = true;
    enemy_hit_timer = 0.f;


    if (health <= 0)
    {
        health = 0;
        dead = true;

        std::cout << "Enemy Dead\n";
    }
}

void Enemy::Draw(sf::RenderWindow& window)
{
    window.draw(enemySprite);
    window.draw(enemyBox);
}