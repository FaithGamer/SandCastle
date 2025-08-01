#pragma once

#include <json/json.hpp>
#include "SandCastle/Core/Log.h"
#include "std_macros.h"

namespace SandCastle
{
	using Json = nlohmann::json;


	/// @brief Serialized version of an object.
	/// Wrapper around json object and fstream functions
	class Serialized
	{
	public:
		Serialized();
		Serialized(String path);
		Serialized(Json&& json, String rpath);

		void LoadFromDisk(String path);
		void SetJson(Json& json);
		void SetJson(Json& json, String rpath);
		void SetJson(Json&& json);
		void SetJson(Json&& json, String rpath);
		void WriteOnDisk(String path);
		/// @brief Set the HadGetError flag to false
		/// useful if you handled the error and want to check for 
		/// other errors.
		void ClearGetError();

		/// @brief Write another serialized into this serialized
		/// @param serialized 
		void AddObj(String name, Serialized& serialized);
		void AddObj(String name, Serialized&& serialized);

		/// @brief Check if an object exists
		/// @param name Name of the object
		/// @return true if it exists
		bool HaveField(String name);
		float GetFloat(String name);
		int64_t GetInt(String name);
		bool GetBool(String name);

		String GetString(String name);
		Serialized GetObj(String name);
		std::vector<Serialized> GetObjArray(String name);
		/// @brief Where this config has been loaded from disk
		/// @return empty string if not loaded from disk
		String GetReadPath() const;
		/// @brief Where this config has been saved on disk for the last time
		/// @return empty string if never written
		String GetWritePath() const;

		/// @brief Return true if it had error when loadinf the file from disk
		bool HadLoadError() const;
		/// @brief Return true if it had error when parsing the file to json
		bool HadParseError() const;
		/// @brief Return true if any of the Get method couldn't find a parameter.
		bool HadGetError() const;

		template <class T>
		bool TryGet(String name, T& value)
		{
			auto it = m_json.find(name);
			if (it != m_json.end())
			{
				try {
					value = it->get<T>();
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
			else
			{
				return false;
			}
		}
		template <class T>
		std::vector<T> GetArray(String name)
		{
			std::vector<T> value;
			if (SafeGet<std::vector<T>>(name, value))
			{
				return value;
			}
			else
			{
				return std::vector<T>();
			}
		}

		inline Json& operator[](String parameter)
		{
			return m_json[parameter];
		}

		inline Serialized& operator=(Json json)
		{
			m_rpath = "";
			m_wpath = "";
			m_hadGetError = false;
			m_json = json;
			return *this;
		}
		inline Serialized& operator=(Serialized rhs)
		{
			m_rpath = "";
			m_wpath = "";
			m_hadGetError = rhs.m_hadGetError;
			m_json = rhs.m_json;
			return *this;
		}
		inline operator Json()
		{
			return m_json;
		}


	private:
		template <class T>
		bool SafeGet(String name, T& value)
		{
			try {
				value = m_json.at(name).get<T>();
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

		bool SafeGetObj(String name, Serialized& value);

		String m_rpath = "";
		String m_wpath = "";
		Json m_json = nullptr;
		bool m_hadGetError = false;
		bool m_hadLoadError = false;
		bool m_hadParseError = false;

		friend void to_json(Json& j, const Serialized& s);
	};

	class Serializable
	{
	public:
		virtual ~Serializable() {};
		virtual void Deserialize(Serialized& config) = 0;
		virtual Serialized Serialize() = 0;
		/// @brief Should return true if the deserialization process hasn't been successful
		virtual bool DeserializationError() { return false; }
	};

	inline void to_json(Json& j, const Serialized& s)
	{
		j = s.m_json;
	}

}