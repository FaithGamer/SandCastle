#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "SandCastle/Core/Vec.h"



namespace SandCastle
{
	class Assets;
	/// @brief GLSL program (vertex + optional geometry + fragment).
	/// Most code uses Material to drive a Shader; direct SetUniform calls are
	/// reserved for low-level rendering. Shaders are compiled by Assets at startup
	/// from the asset folder.
	class Shader
	{
	public:
		Shader() = default;
		/// @brief Compile a vertex+fragment shader pair.
		Shader(std::string vertexSource, std::string fragmentSource, std::string name);
		/// @brief Compile a vertex+geometry+fragment shader trio.
		Shader(std::string vertexSource, std::string geometrySource, std::string fragmentSource, std::string name);
		Shader(const Shader& shader);
		/// @brief Free the underlying GL program.
		void Destroy();

		/// @brief Bind this program for the next draw calls.
		void Bind() const;

		/// @brief Upload a scalar/vec/mat uniform by location.
		void SetUniform(GLint location, const GLfloat& uniform);
		void SetUniform(GLint location, const Vec2f& uniform);
		void SetUniform(GLint location, const Vec3f& uniform);
		void SetUniform(GLint location, const Vec4f& uniform);
		void SetUniform(GLint location, const Mat3& uniform);
		void SetUniform(GLint location, const Mat4& uniform);
		void SetUniform(GLint location, const int& uniform);

		/// @brief Upload an int array uniform.
		void SetUniformArray(GLint location, const int* uniform, GLsizei count);
		/// @brief Upload a float array uniform.
		void SetUniformArray(GLint location, const float* uniform, GLsizei count);

		/// @brief Look up a uniform's location; returns -1 if not present in the program.
		GLint GetUniformLocation(std::string name);
		/// @brief Bind a named uniform block to a binding point (used for UBOs like the scene buffer).
		void BindUniformBlock(std::string uniformName, GLint bindingPoint);

		/// @brief Raw OpenGL program id.
		GLuint GetGLID();
		/// @brief Engine-side stable id (assigned once on creation).
		uint32_t GetID();
		/// @brief Helper: read a shader source file into a string.
		static std::string LoadShaderSourceFromFile(std::string path);
		/// @brief Friendly name (typically the source file stem).
		String GetName();
	private:
		friend Assets;
		static uint32_t m_currentId;
		GLuint m_glid;
		uint32_t m_id;
		String m_name;
	};
}