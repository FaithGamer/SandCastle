#pragma once

#include <vector>
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Render/Shader.h"

namespace SandCastle
{
	class RenderOptions;
	class Renderer2D;
	typedef uint16_t MaterialID;
	struct MaterialProperty
	{
		GLint location;
		GLenum type;
		union {
			Vec4f vec4;
			Vec3f vec3;
			Vec2f vec2;
			float f;
			int i;
		};
	};
	struct MaterialPropertyArray
	{
		GLint location;
		GLenum type;

		std::vector<int> i;
		std::vector<float> f;
	};
	/// @brief A Shader plus a named-uniform value table.
	/// Materials are created by Renderer2D::CreateMaterial and shared between
	/// SpriteRender components / UI elements. Setters return false when the
	/// uniform doesn't exist or has the wrong type.
	class Material
	{
	public:
		/// @brief Set a float uniform by name. Returns false if no such uniform.
		bool SetFloat(String name, float value);
		bool SetInt(String name, int value);
		bool SetVec2f(String name, Vec2f value);
		bool SetVec3f(String name, Vec3f value);
		bool SetVec4f(String name, Vec4f value);
		bool SetFloatArray(String name, const std::vector<float>& value);
		bool SetIntArray(String name, const std::vector<int>& value);

		/// @brief Read a uniform value. Returns the fallback if no such uniform
		/// exists or it has the wrong type. Silent on miss (unlike the setters).
		float GetFloat(const String& name, float fallback = 0.f) const;
		int GetInt(const String& name, int fallback = 0) const;
		Vec2f GetVec2f(const String& name, Vec2f fallback = {0.f, 0.f}) const;
		Vec3f GetVec3f(const String& name, Vec3f fallback = {0.f, 0.f, 0.f}) const;
		Vec4f GetVec4f(const String& name, Vec4f fallback = {0.f, 0.f, 0.f, 0.f}) const;

		/// @brief Direct access to the uniform table for generic iteration
		/// (e.g. dev panels rendering one widget per uniform).
		const std::unordered_map<String, MaterialProperty>& GetProperties() const { return m_properties; }

		/// @brief Bind this material's shader and upload all of its uniform values.
		void Bind() const;
		MaterialID GetID() const;
		Shader* GetShader() const;
		RenderOptions* GetRenderOptions() const;
		/// @brief True if this material is meant to composite a render layer (vs. drawing quads).
		bool IsLayer() const;
	private:
		friend Renderer2D;
		Material(Shader* shader, MaterialID id, bool isLayer);
		void ListUniforms();
		void AddProperty(GLenum type, String name, GLint location);
		void AddPropertyArray(GLenum type, String name, GLint location, GLsizei);
		std::unordered_map<String, MaterialProperty>::iterator FindProperty(const String& name);
		std::unordered_map<String, MaterialPropertyArray>::iterator FindPropertyArray(const String& name);
		//to do, add render options
		std::unordered_map<String, MaterialProperty> m_properties;
		std::unordered_map<String, MaterialPropertyArray> m_arrayProperties;
		Shader* m_shader = nullptr;
		RenderOptions* m_renderOptions = nullptr;
		MaterialID m_id = 0;
		bool m_isLayer = false;
	};
}