#pragma once

#include <SandCastle.h>

struct Dummy
{
	int f;
};
using namespace SandCastle;

std::vector<uint32_t> Utf8ToCodepoints(const std::string& s)
{
	std::vector<uint32_t> cps;
	size_t i = 0, n = s.size();
	while (i < n) {
		uint8_t c = (uint8_t)s[i++];
		if ((c & 0x80) == 0) { cps.push_back(c); continue; }
		if ((c & 0xE0) == 0xC0 && i < n) { cps.push_back(((c & 0x1F) << 6) | (s[i++] & 0x3F)); continue; }
		if ((c & 0xF0) == 0xE0 && i + 1 < n) {
			uint32_t cp = ((c & 0x0F) << 12) | ((s[i] & 0x3F) << 6) | (s[i + 1] & 0x3F);
			i += 2; cps.push_back(cp); continue;
		}
		if ((c & 0xF8) == 0xF0 && i + 2 < n) {
			uint32_t cp = ((c & 0x07) << 18) | ((s[i] & 0x3F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F);
			i += 3; cps.push_back(cp); continue;
		}
		cps.push_back(0xFFFD);
	}
	return cps;
}
class FontTestSys : public System
{
public:
	void Start() override
	{
		auto cps = Utf8ToCodepoints((const char*)u8"mon de");
		for (auto cp : cps)
		{
			LOG_INFO("{0}", cp);
		}

		auto font = Ui::GetWriter();
		auto fontId = font->MakeFont("NotoSansJP-Regular.ttf", 50, 3.f, 1.f, Vec4f(1, 0, 0, 1));
		font->UseFont(fontId);
		auto s = font->Write((const char*)u8"mon de");

		s.root.GetComponent<Transform>()->Move(0, 0, 0);
		s.root.AddComponent<Dummy>();
		auto sprite = s.glyphEntities[0].GetComponent<SpriteRender>()->GetSprite();
		static auto s1 = new Sprite(sprite->GetTexture());

		//Entity::CreateSprite().GetComponent<SpriteRender>()->SetSprite(s1);
		//Entity::CreateSprite();
	}
	void Update() override
	{
		auto delta = Time::Delta();
		auto ranChar = [&]() -> char {
			std::string charset =
				"abcdefghijklmnopqrstuvwxyz"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"0123456789";
			auto c = Random::Range(0, (int)charset.size() - 1);
			return charset[c];
			};

		static float timer = 0.f;

		timer += delta;
		if (timer < 1.f)
			return;
		timer = 0.f;
		Entity::View<Dummy>().each([&](Entity e, Dummy& d)
			{
				e.Destroy();
			});
		std::string str;
		for (int i = 0; i < 14; i++)
		{
			str += ranChar();
		}

		auto e = Ui::GetWriter()->Write(str);
		e.root.AddComponent<Dummy>();
		e.root.gtr()->Move(-e.size.x * 0.5f, 0, 0);

	}
};

void FontTest()
{
	Engine::Init();
	Camera::main->zoom = .04f;
	Systems::Push<FontTestSys>();
	Engine::Launch();
}