#pragma once

#include <vector>
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Render/Shader.h"

namespace SandCastle
{
	class Renderer2D;
	typedef uint32_t MaterialID;
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
	class Material
	{
	public:
		bool SetFloat(String name, float value);
		bool SetInt(String name, int value);
		bool SetVec2f(String name, Vec2f value);
		bool SetVec3f(String name, Vec3f value);
		bool SetVec4f(String name, Vec4f value);
		bool SetFloatArray(String name, const std::vector<float>& value);
		bool SetIntArray(String name, const std::vector<int>& value);
	
		void Bind() const;
		MaterialID GetID() const;
		Shader* GetShader() const;
	private:
		friend Renderer2D;
		Material(Shader* shader, MaterialID);
		void ListUniforms();
		void AddProperty(GLenum type, String name, GLint location);
		void AddPropertyArray(GLenum type, String name, GLint location, GLsizei);
		std::unordered_map<String, MaterialProperty>::iterator FindProperty(const String& name, bool& found);
		std::unordered_map<String, MaterialPropertyArray>::iterator FindPropertyArray(const String& name, bool& found);
		//to do, add render options
		std::unordered_map<String, MaterialProperty> m_properties;
		std::unordered_map<String, MaterialPropertyArray> m_arrayProperties;
		Shader* m_shader;
		MaterialID m_id;
	};
}