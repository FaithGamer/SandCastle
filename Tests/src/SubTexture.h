#pragma once

#include <SandCastle.h>

using namespace SandCastle;

void SubTexture()
{
	Engine::Init();
	Camera::main->zoom = 0.01;
	auto frame = Assets::Get<Sprite>("frame.png_0_0");
	Texture* texture = Renderer2D::CreateSubTexture(frame->GetTexture(), frame->GetTextureRect());
	texture->SetWrapping(TextureWrapping::Repeat);
	Sprite* sprite = new Sprite(texture, Rect(0, 0, 1000, 30));
	Entity e = Entity::CreateSprite();
	e.Get<SpriteRender>()->SetSprite(sprite);
	Engine::Launch();
}

