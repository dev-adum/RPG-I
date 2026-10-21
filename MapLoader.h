#pragma once
#include "MapData.h"
#include <string>

class MapLoader
{
public:
	
	void Load(const std::string& filename, MapData& mapData);
};