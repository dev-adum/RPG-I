#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>
#include "Tile.h"
#include "MapData.h"
#include "MapLoader.h"

class Map
{
public:
    sf::Texture tilesheetTexture;
    std::vector<Tile> tiles;
    MapData md;

    int tileSizeX = 0;
    int tileSizeY = 0;

    Map();
    ~Map();

    void Initialize();
    void Update(float delta_time);
    void Draw(sf::RenderWindow& window);
    void Load(const std::string& mapFile);
    
};