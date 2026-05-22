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

	sf::Texture enemyTexture;
	sf::Sprite enemySprite;
	sf::RectangleShape enemyBox;

	sf::Vector2i enemyBoxSize;
	sf::SoundBuffer hitBuffer1;
	sf::Sound hitSound1;
	
	sf::IntRect frame_1;
	sf::IntRect frame_2;
	sf::IntRect frame_3;
	sf::IntRect frame_4;


	const std::vector<sf::IntRect>* anim = &Idle;

	unsigned int id_idx = 0;


	std::vector<sf::IntRect> Idle;
	
	float enemy_anim_timer = 0.0f;
	float const enemy_anim_speed = 1.f;

	unsigned int* currentIdx = &id_idx;

};