#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

class Enemy
{
public:

	Enemy();

	void Load();
	void Initialize();
	void Update(float delta_time);
	void Draw(sf::RenderWindow& window);

	sf::Texture idleTexture;
	sf::Texture hitTexture;
	sf::Texture stopTexture;
	sf::Sprite enemySprite;
	sf::Sprite bubbleSprite;
	sf::RectangleShape enemyBox;

	sf::Vector2i enemyBoxSize;
	sf::SoundBuffer hitBuffer1;
	sf::Sound hitSound1;
	
	sf::IntRect frame_1;
	sf::IntRect frame_2;
	sf::IntRect frame_3;
	sf::IntRect frame_4;


	const std::vector<sf::IntRect>* idle = &Idle;
	const std::vector<sf::IntRect>* hit = &Hit;

	unsigned int id_idx = 0;
	unsigned int ht_idx = 0;
	unsigned int stop_idx = 0;

	bool isHit = false;
	bool isFiring = false;


	std::vector<sf::IntRect> Idle;
	std::vector<sf::IntRect> Hit;
	std::vector<sf::IntRect> Stop;

	bool showBubble = false;

	
	float bubble_delay = 2.f;

	float bubble_stop_timer = 0.f;
	float bubble_stop_speed = 0.2f;


	float enemy_talk_speed = 0.2f;
	float timeSinceLastShot = 0.f;
	bool showStop = false;


	float enemy_idle_timer = 0.0f;
	float const enemy_idle_speed = 1.f;

	float enemy_state_timer = 0.f;
	float enemy_hit_timer = 0.f;
	float enemy_hit_speed = 0.075f;
	float hit_duration = 0.075f;

	bool bubbleArmed = false;
	float bubble_life_timer = 0.f;
	float bubble_life_duration = 1.5f;
	float bubble_frame_timer = 0.f;
	float bubble_frame_speed = 0.2f;


	unsigned int* currentIdx = &id_idx;

};