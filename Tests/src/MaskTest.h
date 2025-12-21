#pragma once

#include <SandCastle/SandCastle.h>

using namespace SandCastle;

void MaskTest()
{
	Engine::Init();
	auto maskedMat = Renderer2D::CreateMaterial(Assets::Get<Shader>("masked_layer.shader"), true);
	auto maskLayer = Renderer2D::AddOffscreenLayer("mask", 1);
	auto maskedLayer = Renderer2D::AddLayer("masked", maskedMat);

	auto maskedSprite = Entity::CreateSprite("trollface.png_0_0");
	auto rd = maskedSprite.Get<SpriteRender>();
	rd->SetLayer(maskedLayer);

	auto maskSprite = Entity::CreateSprite("trollface.png_0_0");
	maskSprite.gtr()->Move(3, 0, 0);
	auto rdm = maskSprite.Get<SpriteRender>();
	rdm->SetLayer(maskLayer);

	Engine::Launch();
}