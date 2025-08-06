#include "pch.h"
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include "SandCastle/Core/Files.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Render/Shader.h"


#define SET_UNIFORM(c) \
glUseProgram(m_glid);\
c;\


namespace SandCastle
{
	uint32_t Shader::m_currentId = 0;



	void shaderCompilationError(uint32_t shader)
	{
		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE)
		{
			GLint maxLength = 9999;
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
			LOG_ERROR("Shader compilation failed " + std::string(&infoLog[0]));
		}
	}

	void programLinkageError(uint32_t program)
	{
		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);

		if (success == GL_FALSE)
		{
			GLint maxLength = 9999;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			LOG_ERROR("Shader program linkage failed " + std::string(&infoLog[0]));
		}
	}

	/*Shader::Shader(std::string vertexAndFrag)
	{
		String vertex = vertexAndFrag.find_first_of(#fragment)
	}*/

	Shader::Shader(std::string vertexSource, std::string fragmentSource)
	{
		m_id = m_currentId++;
		//Load shader source files
		const GLchar* vertexSrc = (const GLchar*)vertexSource.c_str();
		const GLchar* fragmentSrc = (const GLchar*)fragmentSource.c_str();

		//Creates shaders and compile
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSrc, NULL);
		glCompileShader(vertexShader);
		shaderCompilationError(vertexShader);

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
		glCompileShader(fragmentShader);
		shaderCompilationError(fragmentShader);

		m_glid = glCreateProgram();
		glAttachShader(m_glid, vertexShader);
		glAttachShader(m_glid, fragmentShader);
		glLinkProgram(m_glid);
		programLinkageError(m_glid);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

	}

	Shader::Shader(std::string vertexSource, std::string geometrySource, std::string fragmentSource)
	{
		m_name = 
		m_id = m_currentId++;
		//Load shader source files
		const GLchar* vertexSrc = (const GLchar*)vertexSource.c_str();
		const GLchar* geometrySrc = (const GLchar*)geometrySource.c_str();
		const GLchar* fragmentSrc = (const GLchar*)fragmentSource.c_str();

		//Creates shaders and compile
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSrc, NULL);
		glCompileShader(vertexShader);
		shaderCompilationError(vertexShader);

		GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometrySrc, NULL);
		glCompileShader(geometryShader);
		shaderCompilationError(geometryShader);

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
		glCompileShader(fragmentShader);
		shaderCompilationError(fragmentShader);

		m_glid = glCreateProgram();
		glAttachShader(m_glid, vertexShader);
		glAttachShader(m_glid, geometryShader);
		glAttachShader(m_glid, fragmentShader);
		glLinkProgram(m_glid);
		programLinkageError(m_glid);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

	}

	Shader::~Shader()
	{
		glDeleteProgram(m_glid);
	}

	void Shader::Bind() const
	{
		glUseProgram(m_glid);
	}

	void Shader::SetUniform(GLint location, const GLfloat& uniform)
	{
		SET_UNIFORM(glUniform1f(location, uniform));
	}

	void Shader::SetUniform(GLint location, const Vec2f& uniform)
	{
		SET_UNIFORM(glUniform2f(location, (GLfloat)uniform.x, (GLfloat)uniform.y));
	}

	void Shader::SetUniform(GLint location, const Vec3f& uniform)
	{
		SET_UNIFORM(glUniform3f(location, (GLfloat)uniform.x, (GLfloat)uniform.y, (GLfloat)uniform.z));
	}

	void Shader::SetUniform(GLint location, const Vec4f& uniform)
	{
		SET_UNIFORM(glUniform4f(location, (GLfloat)uniform.x, (GLfloat)uniform.y, (GLfloat)uniform.z, (GLfloat)uniform.w));
	}

	void Shader::SetUniform(GLint location, const Mat3& uniform)
	{
		SET_UNIFORM(glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(uniform)));
	}

	void Shader::SetUniform(GLint location, const Mat4& uniform)
	{
		SET_UNIFORM(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(uniform)));
	}

	void Shader::SetUniform(GLint location, const int& uniform)
	{
		SET_UNIFORM(glUniform1i(location, (GLint)uniform));
	}

	void Shader::SetUniformArray(GLint location, const int* uniform, GLsizei count)
	{
		SET_UNIFORM(glUniform1iv(location, count, uniform));
	}

	void Shader::SetUniformArray(GLint location, const float* uniform, GLsizei count)
	{
		SET_UNIFORM(glUniform1fv(location, count, uniform));
	}

	void Shader::BindUniformBlock(std::string uniformName, GLint bindingPoint)
	{
		GLuint location = glGetUniformBlockIndex(m_glid, (const GLchar*)uniformName.c_str());
		if (location == -1)
		{
			LOG_ERROR("The following uniform block cannot be found: " + uniformName);
		}
		else
		{
			glUniformBlockBinding(m_glid, location, bindingPoint);
		}
	}

	GLint Shader::GetUniformLocation(std::string name)
	{
		return glGetUniformLocation(m_glid, (const GLchar*)name.c_str());
	}

	GLuint Shader::GetGLID()
	{
		return m_glid;
	}

	uint32_t Shader::GetID()
	{
		return m_id;
	}

	std::string Shader::LoadShaderSourceFromFile(std::string path)
	{
		std::ifstream shaderFile;
		shaderFile.open(path);
		if (!shaderFile.is_open())
		{
			LOG_ERROR("Unable to open the shader: " + path);
		}
		return Files::IfstreamToString(shaderFile);
	}
	String Shader::GetName()
	{
		return m_name;
	}
}
