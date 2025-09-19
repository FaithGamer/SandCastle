
#pragma once
#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{
	enum class TextAlign
	{
		Left,
		Center,
		Right
	};

	using FontID = uint32_t;


	struct Sentence
	{
		Entity root;                           // parent entity for the sentence
		std::vector<Entity> glyphEntities;     // child entities (one per glyph)
		Vec2f size;
		float maxWidth = 0.f;
	};
}