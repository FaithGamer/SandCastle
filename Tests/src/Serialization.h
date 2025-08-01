#pragma once

#include <SandCastle/SandCastle.h>

void Save()
{
	Serialized obj;
	Serialized item1, item2, item3;
	std::vector<Serialized> monsters;

	item1["hp"] = 3;
	item1["name"] = "bandit";
	item1["alive"] = true;

	item2["hp"] = 1;
	item2["name"] = "wolf";
	item2["alive"] = true;

	item3["hp"] = 0;
	item3["name"] = "slime";
	item3["alive"] = false;

	monsters.emplace_back(item1);
	monsters.emplace_back(item2);
	monsters.emplace_back(item3);

	obj["monsters"] = monsters;
	obj["biomes"] = std::vector<String>{ "forest", "ocean", "desert" };
	obj["game_count"] = 42;

	obj.WriteOnDisk("data.json");
}
struct Monster
{
	int hp;
	String name;
	bool alive;
};
Monster LoadMonster(Serialized& obj)
{
	Monster monster;
	monster.hp = obj.GetInt("hp");
	monster.name = obj.GetString("name");
	monster.alive = obj.GetBool("alive");
	return monster;
}
void Load()
{
	Serialized obj("data.json");
	std::vector<Serialized> monsters = obj.GetObjArray("monsters");
	for (auto& jmonster : monsters)
	{
		Monster monster = LoadMonster(jmonster);
		LOG_INFO("Loaded monster:\nhp: {0}\nname: {1}\nalive: {2}", monster.hp, monster.name, monster.alive);
	}

	LOG_INFO("biomes:\n");

	for (auto& biome : obj.GetArray<String>("biomes"))
	{
		LOG_INFO("{0}", biome);
	}

	LOG_INFO("game count: {0}", obj.GetInt("game_count"));
}
void Serialization()
{
	Engine::Init();
	Save();
	Load();

}
