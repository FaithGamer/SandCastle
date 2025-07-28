#include "pch.h"
#include "SandCastle/Core/Serialization.h"


namespace SandCastle
{
	Serialized::Serialized() : m_hadGetError(false), m_hadLoadError(false)
	{

	}

	Serialized::Serialized(String path) : Serialized()
	{
		LoadFromDisk(path);
	}

	void Serialized::LoadFromDisk(String path)
	{
		m_hadLoadError = false;

		m_rpath = path;
		auto file = std::ifstream(path);
		if (!file.is_open())
		{
			m_hadLoadError = true;
			LOG_ERROR("Cannot load Serialized from path: " + path);
			return;
		}
		try {

			file >> m_json;
		}
		catch (Json::exception& exception)
		{
			m_hadLoadError = true;
			String errorMsg = "Serialized error when parsing: " + path + "\n";
			errorMsg += "json exception: " + std::to_string(exception.id) + ", ";
			errorMsg += *exception.what() + "\n";

			LOG_ERROR(errorMsg);
		}

	}
	void Serialized::SetJson(Json& json)
	{
		m_hadGetError = false;
		m_rpath = "child_object";
		m_wpath = "";
		m_json = json;
	}
	void Serialized::SetJson(Json& json, String rpath)
	{
		m_hadGetError = false;
		m_rpath = rpath;
		m_wpath = "";
		m_json = json;
	}
	void Serialized::SetJson(Json&& json)
	{
		m_hadGetError = false;
		m_rpath = "child_object";
		m_wpath = "";
		m_json = json;
	}
	void Serialized::SetJson(Json&& json, String rpath)
	{
		m_hadGetError = false;
		m_rpath = rpath;
		m_wpath = "";
		m_json = json;
	}
	void Serialized::WriteOnDisk(String path)
	{
		auto file = std::ofstream(path);
		if (!file.is_open())
		{
			LOG_ERROR("Cannot write config to path: " + path);
			return;
		}
		try {

			file << m_json;
		}
		catch (Json::exception& exception)
		{
			String errorMsg = "Serialized error when writing: " + path + "\n";
			errorMsg += "json exception: " + std::to_string(exception.id) + ", ";
			errorMsg += *exception.what() + "\n";

			LOG_ERROR(errorMsg);
		}
		file.close();
	}
	void Serialized::AddObj(String name, Serialized& serialized)
	{
		m_json[name] = serialized.m_json;
	}
	void Serialized::AddObj(String name, Serialized&& serialized)
	{
		m_json[name] = serialized.m_json;
	}
	bool Serialized::HaveField(String name)
	{
		return m_json.find(name) != m_json.end();
	}
	float Serialized::GetFloat(String name)
	{
		float value;
		if (SafeGet<float>(name.c_str(), value))
		{
			return value;
		}
		else
		{
			return 0.0f;
		}
	}

	int64_t Serialized::GetInt(String name)
	{
		int64_t value;
		if (SafeGet<int64_t>(name, value))
		{
			return value;
		}
		else
		{
			return 0;
		}
	}

	bool Serialized::GetBool(String name)
	{
		bool value;
		if (SafeGet<bool>(name, value))
		{
			return value;
		}
		else
		{
			return false;
		}
	}

	String Serialized::GetString(String name)
	{
		String  value;
		if (SafeGet<String>(name, value))
		{
			return value;
		}
		else
		{
			return "undefined";
		}
	}

	Serialized Serialized::GetObj(String name)
	{
		Serialized value;
		if (SafeGetObj(name, value))
		{
			value.m_rpath = m_rpath + "->" + name;
			return value;
		}
		else
		{
			Serialized illFormed;
			illFormed.m_hadGetError = true;
			return illFormed;
		}
	}
	bool Serialized::SafeGetObj(String name, Serialized& value)
	{
		{
			try {
				value = m_json.at(name);
				value.m_rpath = m_rpath;
				return true;
			}
			catch (Json::exception& exception)
			{
				String errorMsg = "Serialized error in file " + m_rpath + ", parameter: " + String(name) + "\n";
				errorMsg += "json exception: " + std::to_string(exception.id) + ", ";
				errorMsg += String(exception.what()) + "\n";
				m_hadGetError = true;
				LOG_ERROR(errorMsg);

				return false;
			}
		}
	}

	String Serialized::GetReadPath() const
	{
		return m_wpath;
	}
	String Serialized::GetWritePath() const
	{
		return m_wpath;
	}

	bool Serialized::HadGetError() const
	{
		return m_hadGetError;
	}

	bool Serialized::HadLoadError() const
	{
		return m_hadLoadError;
	}
	
}