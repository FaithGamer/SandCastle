#include "pch.h"
#include "SandCastle/Render/Material.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	Material::Material(Shader* shader, MaterialID id) : m_shader(shader), m_id(id)
	{
		ListUniforms();
	}
	void SandCastle::Material::Bind() const
	{
		for (auto& prop_kvp : m_properties)
		{
			switch (prop_kvp.second.type)
			{
			case GL_FLOAT_VEC2:
				m_shader->SetUniform(prop_kvp.second.location, prop_kvp.second.vec2);
				break;
			case GL_FLOAT_VEC3:
				m_shader->SetUniform(prop_kvp.second.location, prop_kvp.second.vec3);
				break;
			case GL_FLOAT_VEC4:
				m_shader->SetUniform(prop_kvp.second.location, prop_kvp.second.vec4);
				break;
			case GL_FLOAT:
				m_shader->SetUniform(prop_kvp.second.location, prop_kvp.second.f);
				break;
			case GL_INT:
				m_shader->SetUniform(prop_kvp.second.location, prop_kvp.second.i);
				break;
			}
		}
		for (auto& prop_kvp : m_arrayProperties)
		{
			switch (prop_kvp.second.type)
			{
			case GL_INT:
				m_shader->SetUniformArray(prop_kvp.second.location, &prop_kvp.second.i[0], prop_kvp.second.i.size());
				break;
			case GL_FLOAT_VEC3:
				m_shader->SetUniformArray(prop_kvp.second.location, &prop_kvp.second.f[0], prop_kvp.second.f.size());
				break;
			}
		}
		m_shader->Bind();
	}
	MaterialID Material::GetID() const
	{
		return m_id;
	}
	Shader* Material::GetShader() const
	{
		return m_shader;
	}
	void Material::ListUniforms()
	{
		//List the existing uniforms
		GLint numUniforms = 0;
		auto shaderID = m_shader->GetGLID();
		glGetProgramiv(shaderID, GL_ACTIVE_UNIFORMS, &numUniforms);

		for (GLint i = 0; i < numUniforms; ++i) {
			char nameBuffer[256];
			GLsizei length = 0;
			GLint size = 0;
			GLenum type = 0;


			glGetActiveUniform(shaderID, i, sizeof(nameBuffer), &length, &size, &type, nameBuffer);

			std::string name(nameBuffer, length);

			if (size > 1) {
				size_t pos = name.find("[0]");
				if (pos != std::string::npos) {
					name = name.substr(0, pos);
				}
			}

			GLint location = glGetUniformLocation(shaderID, name.c_str());

			if (location == -1)
				continue;


			if (size > 1)
				AddProperty(type, name, location);
			else
				AddPropertyArray(type, name, location, size);
		}
	}
	void Material::AddProperty(GLenum type, String name, GLint location)
	{
		switch (type)
		{
		case GL_INT:
		{
			MaterialProperty property(location, GL_INT, { .i = 0 });
			m_properties.insert(std::make_pair(name, property));
		}
		break;
		case GL_FLOAT:
		{
			MaterialProperty property(location, GL_FLOAT, { .f = 0.f });
			m_properties.insert(std::make_pair(name, property));
		}
		break;
		case GL_FLOAT_VEC2:
		{
			MaterialProperty property(location, GL_FLOAT_VEC2, { .vec2 = {0, 0} });
			m_properties.insert(std::make_pair(name, property));
		}
		break;
		case GL_FLOAT_VEC3:
		{
			MaterialProperty property(location, GL_FLOAT_VEC3, { .vec3 = {0, 0, 0} });
			m_properties.insert(std::make_pair(name, property));
		}
		break;
		case GL_FLOAT_VEC4:
		{
			MaterialProperty property(location, GL_FLOAT_VEC4, { .vec4 = {0, 0, 0, 0} });
			m_properties.insert(std::make_pair(name, property));
		}
		break;
		default:
			LOG_ERROR("Unsupported uniform type in shader {0} type GLenum: {1}", m_shader->GetName(), type);
			break;
		}
	}

	void Material::AddPropertyArray(GLenum type, String name, GLint location, GLsizei size)
	{
		MaterialPropertyArray property;
		switch (type)
		{
		case GL_INT:
			property.type = GL_INT;
			property.location = location;
			property.i.reserve(size);
			for (int i = 0; i < (int)size; i++)
			{
				property.i[i] = 0;
			}
			m_arrayProperties.insert(std::make_pair(name, property));
			break;
		case GL_FLOAT:
			property.type = GL_FLOAT;
			property.location = location;
			property.f.reserve(size);
			for (int i = 0; i < (int)size; i++)
			{
				property.f[i] = 0.f;
			}
			m_arrayProperties.insert(std::make_pair(name, property));
			break;
		default:
			LOG_ERROR("Unsupported uniform-array type in shader {0} type GLenum: {1}", m_shader->GetName(), type);
			break;
		}

	}
	std::unordered_map<String, MaterialProperty>::iterator Material::FindProperty(const String& name, bool& found)
	{
		auto it_prop = m_properties.find(name);
		if (it_prop == m_properties.end())
		{
			LOG_ERROR("The property {0}, doesn't exists for the shader {1}", name, m_shader->GetName());
			found = false;
		}
		else
		{
			found = true;
			return it_prop;
		}
	}
	std::unordered_map<String, MaterialPropertyArray>::iterator Material::FindPropertyArray(const String& name, bool& found)
	{
		auto it_prop = m_arrayProperties.find(name);
		if (it_prop == m_arrayProperties.end())
		{
			LOG_ERROR("The property array {0}, doesn't exists for the shader {1}", name, m_shader->GetName());
			found = false;
		}
		else
		{
			found = true;
			return it_prop;
		}
	}
	bool Material::SetFloat(String name, float value)
	{
		bool r = true;
		auto it_prop = FindProperty(name, r);
		if (!r)
			return false;
		if (it_prop->second.type != GL_FLOAT)
		{
			LOG_ERROR("The property {0}, in the shader {1}, is not of type float", name, m_shader->GetName());
			return false;
		}
		it_prop->second.f = value;
		return true;
	}
	bool Material::SetInt(String name, int value)
	{
		bool r = true;
		auto it_prop = FindProperty(name, r);
		if (!r)
			return false;
		if (it_prop->second.type != GL_INT)
		{
			LOG_ERROR("The property {0}, in the shader {1}, is not of type float", name, m_shader->GetName());
			return false;
		}
		it_prop->second.i = value;
		return true;
	}
	bool Material::SetVec2f(String name, Vec2f value)
	{
		bool r = true;
		auto it_prop = FindProperty(name, r);
		if (!r)
			return false;
		if (it_prop->second.type != GL_FLOAT_VEC2)
		{
			LOG_ERROR("The property {0}, in the shader {1}, is not of type vec2", name, m_shader->GetName());
			return false;
		}
		it_prop->second.vec2 = value;
		return true;
	}
	bool Material::SetVec3f(String name, Vec3f value)
	{
		bool r = true;
		auto it_prop = FindProperty(name, r);
		if (!r)
			return false;
		if (it_prop->second.type != GL_FLOAT_VEC3)
		{
			LOG_ERROR("The property {0}, in the shader {1}, is not of type vec3", name, m_shader->GetName());
			return false;
		}
		it_prop->second.vec3 = value;
		return true;
	}
	bool Material::SetVec4f(String name, Vec4f value)
	{
		bool r = true;
		auto it_prop = FindProperty(name, r);
		if (!r)
			return false;
		if (it_prop->second.type != GL_FLOAT_VEC4)
		{
			LOG_ERROR("The property {0}, in the shader {1}, is not of type vec4", name, m_shader->GetName());
			return false;
		}
		it_prop->second.vec4 = value;
		return true;
	}

	bool Material::SetFloatArray(String name, const std::vector<float>& value)
	{
		bool r = true;
		auto it_prop = FindPropertyArray(name, r);
		if (!r)
			return false;
		if (it_prop->second.type != GL_FLOAT)
		{
			LOG_ERROR("The property array {0}, in the shader {1}, is not of type float", name, m_shader->GetName());
			return false;
		}
		it_prop->second.f = value;
		return true;
	}
	bool Material::SetIntArray(String name, const std::vector<int>& value)
	{
		bool r = true;
		auto it_prop = FindPropertyArray(name, r);
		if (!r)
			return false;
		if (it_prop->second.type != GL_INT)
		{
			LOG_ERROR("The property array {0}, in the shader {1}, is not of type int", name, m_shader->GetName());
			return false;
		}
		it_prop->second.i = value;
		return true;
	}
}