#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>


class Player
{
public:

	Player();

	void Load();
	void Initialize();
	void Update(sf::Vector2f dir, sf::Vector2f aimDir, float delta_time);
	

	sf::Vector2f GetClampedAim(sf::RenderWindow& window, float maxDistance);

	sf::Texture playerTexture;
	sf::Sprite playerSprite;

	sf::Vector2f pos;

	sf::Texture targetTexture;
	sf::Sprite targetSprite;

	sf::SoundBuffer walkBuffer;
	sf::SoundBuffer fireBuffer1;
	sf::SoundBuffer fireBuffer2;
	sf::Sound fireSound1;
	sf::Sound fireSound2;
	sf::Music VoidTrack;
	sf::Sound walkSound;

	sf::IntRect frame_1;
	sf::IntRect frame_2;
	sf::IntRect frame_3;
	sf::IntRect frame_4;

	sf::IntRect frame_5;
	sf::IntRect frame_6;
	sf::IntRect frame_7;
	sf::IntRect frame_8;

	sf::IntRect frame_9;
	sf::IntRect frame_10;
	sf::IntRect frame_11;
	sf::IntRect frame_12;

	sf::IntRect frame_13;
	sf::IntRect frame_14;
	sf::IntRect frame_15;
	sf::IntRect frame_16;

	std::vector<sf::IntRect> SouthWest;
	std::vector<sf::IntRect> South;
	std::vector<sf::IntRect> SouthEast;
	std::vector<sf::IntRect> East;
	std::vector<sf::IntRect> NorthEast;
	std::vector<sf::IntRect> North;
	std::vector<sf::IntRect> NorthWest;
	std::vector<sf::IntRect> West;

	std::vector<sf::IntRect> Target;
	const std::vector<sf::IntRect>* anim = &South;


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
	unsigned int* currentIdx = &s_idx;


};