#pragma once

#include <SandCastle.h>

using namespace SandCastle;
class FontTestSys : public System
{
public:
	void OnStart() override
	{
		auto font = Systems::Get<FontSystem>();
		auto fontId = font->MakeFont("NotoSansJP-Regular.ttf", 50, 1);
		font->UseFont(fontId);
		auto s = font->Write((const char*)u8"Bonjour tout le monde d d d i");
		s.root.GetComponent<Transform>()->Move(0, 0, -1);
		//Entity::CreateSprite();
	}
};

void FontTest()
{
	Engine::Init();
	Systems::Push<FontTestSys>();
	Engine::Launch();
}