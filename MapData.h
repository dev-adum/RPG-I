#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <SFML/Graphics.hpp>

struct MapObject {
	std::string name;       // e.g., "Door", "Chest", "NPC"
	sf::FloatRect bounds;   // Rectangle bounds from Tiled

	// Optional properties (used depending on what the object is)
	std::string targetMap;
	sf::Vector2f spawnPos;
};

struct MapData
{
private:
	
	std::string m_tilesheet = "";
	std::string m_name = "";
	std::string m_version = "0";
	std::vector<MapObject> m_objects;

	int m_mapPositionX = 0;
	int m_mapPositionY = 0;

	int m_tileSizeX = 0;
	int m_tileSizeY = 0;

	int m_totalTilesX = 0;
	int m_totalTilesY = 0;

	int m_mapScaleX = 0;
	int m_mapScaleY = 0;

	std::vector<std::vector<int>> m_data;
	
	std::unordered_map<int, bool> m_solidTiles;
	std::unordered_map<int, bool> m_doorTiles;
public:
	MapData() = default;

	MapData(const std::string& tilesheet, const std::string& name, const std::string& version,
		int mapPositionX, int mapPositionY, int tileSizeX, int tileSizeY,
		int totalTilesX, int totalTilesY, int mapScaleX, int mapScaleY,
		const std::vector<std::vector<int>>& data)
	{
		m_data = data;
		m_tilesheet = tilesheet;
		m_version = version;
		m_name = name;

		m_mapPositionX = mapPositionX;
		m_mapPositionY = mapPositionY;

		m_tileSizeX = tileSizeX;
		m_tileSizeY = tileSizeY;

		m_totalTilesX = totalTilesX;
		m_totalTilesY = totalTilesY;

		m_mapScaleX = mapScaleX;
		m_mapScaleY = mapScaleY;
	}

	// Getters
	inline const std::vector<std::vector<int>>& GetData() const { return m_data; }
	inline const std::string& GetTilesheet() const { return m_tilesheet; }
	inline const std::string& GetName() const { return m_name; }

	inline const std::string& GetVersion() const { return m_version; }
	inline void AddObject(const MapObject& obj) {m_objects.push_back(obj);}

	inline int GetMapPositionX() const { return m_mapPositionX; }
	inline int GetMapPositionY() const { return m_mapPositionY; }

	inline int GetTileSizeX() const { return m_tileSizeX; }
	inline int GetTileSizeY() const { return m_tileSizeY; }

	inline int GetTotalTilesX() const { return m_totalTilesX; }
	inline int GetTotalTilesY() const { return m_totalTilesY; }

	inline int GetMapScaleX() const { return m_mapScaleX; }
	inline int GetMapScaleY() const { return m_mapScaleY; }

	inline int GetDataLength() const {
		return static_cast<int>(m_data.size());
	}
	inline bool IsTileSolid(int globalId) const {
		auto it = m_solidTiles.find(globalId);
		if (it != m_solidTiles.end()) {
			return it->second;
		}
		return false; // Default to walkable if no property is set
	
	}

	inline bool IsTileDoor(int globalId) const {
		auto it = m_doorTiles.find(globalId);
		if (it != m_doorTiles.end()) {
			return it->second;
		}
		return false;
	}

	// Setters
	inline void SetTileDoor(int globalId, bool isDoor) {m_doorTiles[globalId] = isDoor;}
	inline void SetTilesheet(const std::string& tilesheet) { m_tilesheet = tilesheet; }
	inline void SetName(const std::string& name) { m_name = name; }
	inline void SetVersion(const std::string& version) { m_version = version; }
	inline const std::vector<MapObject>& GetObjects() const {return m_objects;}

	inline void SetMapPositionX(int v) { m_mapPositionX = v; }
	inline void SetMapPositionY(int v) { m_mapPositionY = v; }

	inline void SetTileSizeX(int v) { m_tileSizeX = v; }
	inline void SetTileSizeY(int v) { m_tileSizeY = v; }

	inline void SetTotalTilesX(int v) { m_totalTilesX = v; }
	inline void SetTotalTilesY(int v) { m_totalTilesY = v; }

	inline void SetMapScaleX(int v) { m_mapScaleX = v; }
	inline void SetMapScaleY(int v) { m_mapScaleY = v; }

	inline void SetData(const std::vector<std::vector<int>>& data) { m_data = data; }

	inline void SetTileSolid(int globalId, bool isSolid) {
		m_solidTiles[globalId] = isSolid;
	}
};
