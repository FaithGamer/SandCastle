#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "SandCastle/Core/Vec.h"



namespace SandCastle
{
	class Assets;
	class Shader
	{
	public:
		Shader(std::string vertexSource, std::string fragmentSource);
		Shader(std::string vertexSource, std::string geometrySource, std::string fragmentSource);
		~Shader();

		void Bind() const;

		void SetUniform(GLint location, const GLfloat& uniform);
		void SetUniform(GLint location, const Vec2f& uniform);
		void SetUniform(GLint location, const Vec3f& uniform);
		void SetUniform(GLint location, const Vec4f& uniform);
		void SetUniform(GLint location, const Mat3& uniform);
		void SetUniform(GLint location, const Mat4& uniform);
		void SetUniform(GLint location, const int& uniform);

		void SetUniformArray(GLint location, const int* uniform, GLsizei count);
		void SetUniformArray(GLint location, const float* uniform, GLsizei count);

		GLint GetUniformLocation(std::string name);
		void BindUniformBlock(std::string uniformName, GLint bindingPoint);

		GLuint GetGLID();
		uint32_t GetID();
		static std::string LoadShaderSourceFromFile(std::string path);
		String GetName();
	private:
		friend Assets;
		static uint32_t m_currentId;
		GLuint m_glid;
		uint32_t m_id;
		String m_name;
	};
}