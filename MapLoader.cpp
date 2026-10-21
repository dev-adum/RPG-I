#include "MapLoader.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

void MapLoader::Load(const std::string& filename, MapData& mapData)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "Unable to open Tiled JSON file: " << filename << std::endl;
        return;
    }

    try
    {
        nlohmann::json jsonMap;
        file >> jsonMap;

        // 1. Basic Metadata
        if (jsonMap.contains("width"))
            mapData.SetTotalTilesX(jsonMap["width"]);
        if (jsonMap.contains("height"))
            mapData.SetTotalTilesY(jsonMap["height"]);
        if (jsonMap.contains("tilewidth"))
            mapData.SetTileSizeX(jsonMap["tilewidth"]);
        if (jsonMap.contains("tileheight"))
            mapData.SetTileSizeY(jsonMap["tileheight"]);
        if (jsonMap.contains("version"))
            mapData.SetVersion(jsonMap["version"].is_string() ? jsonMap["version"].get<std::string>() : std::to_string(jsonMap["version"].get<int>()));

        // 2. Tilesheet Source Path
        size_t lastSlash = filename.find_last_of("/\\");
        std::string folderPath = (lastSlash != std::string::npos) ? filename.substr(0, lastSlash + 1) : "";

        if (jsonMap.contains("tilesets") && !jsonMap["tilesets"].empty())
        {
            auto& tileset = jsonMap["tilesets"][0];
            if (tileset.contains("image"))
            {
                std::string imagePath = tileset["image"];
                mapData.SetTilesheet(folderPath + imagePath);
            }
        }

        // 3. Parse Tile Custom Properties (like isSolid) from all tilesets
        if (jsonMap.contains("tilesets"))
        {
            for (const auto& tilesetJson : jsonMap["tilesets"])
            {
                int firstGid = tilesetJson["firstgid"].get<int>();

                if (tilesetJson.contains("tiles"))
                {
                    for (const auto& tileJson : tilesetJson["tiles"])
                    {
                        int localId = tileJson["id"].get<int>(); // 0-based index in the tileset
                        int globalId = localId + firstGid;       // Matches the ID used in your map grid

                        if (tileJson.contains("properties"))
                        {
                            for (const auto& prop : tileJson["properties"])
                            {
                                if (prop["name"] == "isSolid" && prop["type"] == "bool")
                                {
                                    bool solid = prop["value"].get<bool>();
                                    mapData.SetTileSolid(globalId, solid);
                                }

                                else if (prop["name"] == "isDoor" && prop["type"] == "bool")
                                {
                                    bool isDoor = prop["value"].get<bool>();
                                    mapData.SetTileDoor(globalId, isDoor);
                                }

                            }
                        }
                    }
                }
            }
        }

        // --- 4. PARSE OBJECT LAYERS (Doors, Triggers, Spawn Zones) ---
        if (jsonMap.contains("layers"))
        {
            for (const auto& layer : jsonMap["layers"])
            {
                std::string layerType = layer.value("type", "");

                if (layerType == "objectgroup")
                {
                    if (layer.contains("objects"))
                    {
                        for (const auto& objJson : layer["objects"])
                        {
                            MapObject mapObj;

                            mapObj.name = objJson.value("name", "Unnamed");

                            float posX = objJson.value("x", 0.f);
                            float posY = objJson.value("y", 0.f);
                            float width = objJson.value("width", 0.f);
                            float height = objJson.value("height", 0.f);

                            mapObj.bounds = sf::FloatRect({ posX, posY }, { width, height });

                            if (objJson.contains("properties"))
                            {
                                for (const auto& prop : objJson["properties"])
                                {
                                    std::string propName = prop.value("name", "");

                                    if (propName == "targetMap")
                                    {
                                        mapObj.targetMap = prop.value("value", "");
                                    }
                                    else if (propName == "spawnX")
                                    {
                                        mapObj.spawnPos.x = prop.value("value", 0.f);
                                    }
                                    else if (propName == "spawnY")
                                    {
                                        mapObj.spawnPos.y = prop.value("value", 0.f);
                                    }
                                }
                            }

                            mapData.AddObject(mapObj);
                        }
                    }
                }
            }
        }

        // 5. Grid Data (Converting 1D flat array from Tiled into your 2D vector grid)
        if (jsonMap.contains("layers") && !jsonMap["layers"].empty())
        {
            // Note: If your tile layer isn't always layers[0] because of object layers, 
            // you can loop through layers to find the one where type == "tilelayer".
            // For now, looking at your setup, finding the tilelayer specifically is safest:
            for (const auto& layer : jsonMap["layers"])
            {
                if (layer.value("type", "") == "tilelayer" && layer.contains("data"))
                {
                    std::vector<int> flatData = layer["data"].get<std::vector<int>>();
                    int width = mapData.GetTotalTilesX();

                    std::vector<std::vector<int>> rows;
                    if (width > 0 && !flatData.empty())
                    {
                        std::vector<int> currentRow;
                        for (size_t i = 0; i < flatData.size(); ++i)
                        {
                            currentRow.push_back(flatData[i]);

                            if ((i + 1) % width == 0)
                            {
                                rows.push_back(currentRow);
                                currentRow.clear();
                            }
                        }
                    }

                    mapData.SetData(rows);
                    break; // Found our tile layer, stop searching
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Failed to parse Tiled JSON map: " << e.what() << std::endl;
    }
}