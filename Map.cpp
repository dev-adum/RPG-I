#include "Map.h"

Map::Map()
{
}

Map::~Map()
{
}

void Map::Initialize()
{
    Load("Assets/Map/Westarch/WA_A1.tmj");
}

void Map::Update(float delta_time)
{
    (void)delta_time;
}

void Map::Load(const std::string& mapFile)
{
    MapLoader loader;
    loader.Load(mapFile, md);

    if (!tilesheetTexture.loadFromFile(md.GetTilesheet()))
    {
        std::cout << "Tilesheet failed to load: " << md.GetTilesheet() << std::endl;
        return;
    }
    
    std::cout << "Tilesheet loaded: " << md.GetTilesheet() << std::endl;

    tileSizeX = md.GetTileSizeX();
    tileSizeY = md.GetTileSizeY();

    unsigned int columns = tilesheetTexture.getSize().x / tileSizeX;
    unsigned int rows = tilesheetTexture.getSize().y / tileSizeY;

    tiles.clear();
    int currentId = 0;
    for (unsigned int row = 0; row < rows; row++)
    {
        for (unsigned int col = 0; col < columns; col++)
        {
            Tile tile;
            tile.id = currentId++;
            tile.position = { static_cast<int>(col * tileSizeX),
                              static_cast<int>(row * tileSizeY) };

            tile.sprite.emplace(tilesheetTexture);
            tile.sprite->setTextureRect(sf::IntRect(tile.position, { tileSizeX, tileSizeY }));

            tiles.push_back(tile);
        }
    }
    std::cout << "Tiles created: " << tiles.size() << std::endl;
}

void Map::Draw(sf::RenderWindow& window)
{
    const auto& layout = md.GetData();
    for (size_t row = 0; row < layout.size(); row++)
    {
        for (size_t col = 0; col < layout[row].size(); col++)
        {
            int tileId = layout[row][col];
            if (tileId > 0 && (tileId -1) < static_cast<int>(tiles.size()))
            {
                Tile& tile = tiles[tileId - 1];
                tile.sprite->setPosition({ static_cast<float>(col * tileSizeX),
                                           static_cast<float>(row * tileSizeY) });
                window.draw(*tile.sprite);
            }
        }
    }
}