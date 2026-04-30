
#pragma once
#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{
	/// @brief Horizontal alignment for laid-out text.
	enum class TextAlign
	{
		Left,
		Center,
		Right
	};

	/// @brief Stable identifier of a font registered with Writer.
	using FontID = uint32_t;


	/// @brief Result of a Writer::Write call: a root entity holding one child per glyph.
	/// Move/destroy the root to move/destroy the whole sentence at once.
	struct Sentence
	{
		Entity root;                           // parent entity for the sentence
		std::vector<Entity> glyphEntities;     // child entities (one per glyph)
		Vec2f size;
		float maxWidth = 0.f;
	};
}