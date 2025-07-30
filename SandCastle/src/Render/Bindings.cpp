#include "pch.h"
#include "SandCastle/Input/Bindings.h"

namespace SandCastle
{
	void Button::Deserialize(Serialized& config)
	{
		mouse = (Mouse::Button)config.GetInt("mouse");
		key = (Key::Scancode)config.GetInt("key");
		controller = (Gamepad::Button)config.GetInt("controller");
		trigger = (Gamepad::Trigger)config.GetInt("trigger");
	}

	Serialized Button::Serialize()
	{
		Serialized config;

		config["mouse"] = (int)mouse;
		config["key"] = (int)key;
		config["controller"] = (int)controller;
		config["trigger"] = (int)trigger;

		return config;
	}

	void ButtonBindings::Deserialize(Serialized& config)
	{
		auto jbuttons = config.GetArray<Json>("buttons");

		for (auto& jbutton : jbuttons)
		{
			buttons.push_back(Button());
			Serialized buttonConfig;
			buttonConfig.SetJson(jbutton);
			buttons.back().Deserialize(buttonConfig);
		}
	}

	Serialized ButtonBindings::Serialize()
	{
		Serialized config;
		std::vector<Json> buttonsj;
		for (auto& button : buttons)
		{
			buttonsj.push_back(button.Serialize());
		}
		config["buttons"] = buttonsj;

		return config;
	}
}