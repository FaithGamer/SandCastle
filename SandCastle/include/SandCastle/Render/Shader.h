#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace SandCastle
{
	class Shader
	{
	public:
		Shader(std::string vertexSource, std::string fragmentSource);
		Shader(std::string vertexSource, std::string geometrySource, std::string fragmentSource);
		~Shader();

		void Bind() const;

		//To do: convert glm vec to sanbox vec
		void SetUniform(std::string name, const GLfloat& uniform);
		void SetUniform(std::string name, const glm::vec2& uniform);
		void SetUniform(std::string name, const glm::vec3& uniform);
		void SetUniform(std::string name, const glm::vec4& uniform);
		void SetUniform(std::string name, const glm::mat3& uniform);
		void SetUniform(std::string name, const glm::mat4& uniform);
		void SetUniform(std::string name, const GLint& uniform);
		void SetUniform(std::string name, const glm::i32vec2& uniform);
		void SetUniform(std::string name, const glm::i32vec3& uniform);
		void SetUniform(std::string name, const glm::i32vec4& uniform);

		void SetUniform(GLint location, const GLfloat& uniform);
		void SetUniform(GLint location, const glm::vec2& uniform);
		void SetUniform(GLint location, const glm::vec3& uniform);
		void SetUniform(GLint location, const glm::vec4& uniform);
		void SetUniform(GLint location, const glm::mat3& uniform);
		void SetUniform(GLint location, const glm::mat4& uniform);
		void SetUniform(GLint location, const GLint& uniform);
		void SetUniform(GLint location, const glm::i32vec2& uniform);
		void SetUniform(GLint location, const glm::i32vec3& uniform);
		void SetUniform(GLint location, const glm::i32vec4& uniform);

		void SetUniformArray(std::string name, const int* uniform, GLsizei count);
		void SetUniformArray(std::string name, const float* uniform, GLsizei count);

		GLint GetUniformLocation(std::string name);
		void BindUniformBlock(std::string uniformName, GLint bindingPoint);

		GLuint GetGLID();
		uint32_t GetID();
		static std::string LoadShaderSourceFromFile(std::string path);
	private:
		static uint32_t m_currentId;
		GLuint m_glid;
		uint32_t m_id;
	};
}