#include "pch.h"
#include "SandCastle/Render/TextureSettings.h"

namespace SandCastle
{
	TextureImportSettings TextureImportSettings::defaultSettings = TextureImportSettings();
	TextureImportSettings::TextureImportSettings(TextureFiltering Filtering, TextureWrapping Wrapping, float PixelPerUnit, bool UseMipmaps, bool KeepData)
		: filtering(Filtering), wrapping(Wrapping), pixelPerUnit(PixelPerUnit), useMipmaps(UseMipmaps), keepData(KeepData)
	{

	}
	TextureImportSettings::TextureImportSettings(Serialized& parameters) : TextureImportSettings()
	{
		Deserialize(parameters);
	}

	void TextureImportSettings::Deserialize(Serialized& parameters)
	{
		String Filtering = parameters.GetString("Filtering");
		if (Filtering == "Linear")
			filtering = TextureFiltering::Linear;
		else if (Filtering == "Nearest")
			filtering = TextureFiltering::Nearest;
		else
		{
			LOG_WARN("Unknown texture filtering parameters: " + Filtering + ", in file: " + parameters.GetReadPath());
		}

		String Wrapping = parameters.GetString("Wrapping");
		if (Wrapping == "Clamp")
			wrapping = TextureWrapping::Clamp;
		else if (Wrapping == "Repeat")
			wrapping = TextureWrapping::Repeat;
		else
		{
			LOG_WARN("Unknown texture wrapping parameters: " + Wrapping + ", in file: " + parameters.GetReadPath());
		}

		pixelPerUnit = parameters.GetFloat("PixelPerUnit");
		useMipmaps = parameters.GetBool("Mipmaps");
		keepData = parameters.GetBool("KeepData");
		if (parameters.HaveField("LodMin"))
			lodMin = parameters.GetInt("LodMin");
		if (parameters.HaveField("LodMax"))
			lodMax = parameters.GetInt("LodMax");
		valid = parameters.HadGetError();
	}
	Serialized TextureImportSettings::Serialize()
	{
		Serialized settings;
		if (filtering == TextureFiltering::Linear)
			settings["Filtering"] = "Linear";
		else
			settings["Filtering"] = "Nearest";

		if (wrapping == TextureWrapping::Clamp)
			settings["Wrapping"] = "Clamp";
		else
			settings["Wrapping"] = "Repeat";

		settings["Mipmaps"] = useMipmaps;
		settings["KeepData"] = keepData;
		settings["PixelPerUnit"] = pixelPerUnit;

		return settings;
	}

	bool TextureImportSettings::DeserializationError()
	{
		return valid;
	}
}