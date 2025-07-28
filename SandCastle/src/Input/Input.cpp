#include "pch.h"
#include "SandCastle/Input/Input.h"
#include "SandCastle/Input/InputMap.h"
#include "SandCastle/Input/ButtonInput.h"

namespace SandCastle
{

	std::string InputTypeName(InputType type)
	{
		switch (type)
		{
		case InputType::Button:			return "Button";		break;
		case InputType::Directional:	return "Directional";	break;
		case InputType::Textual:		return "Textual";		break;
		default:
			LOG_WARN("Trying to get the name of an incorrect InputType");
			return "TypeError";
			break;
		}
	}

	/* ____ Input ____ */

	void Input::OnEventListenedUpdated()
	{
		if (m_inputMap == nullptr)
		{
			LOG_ERROR("Input has no InputMap, Input: " + GetName());
		}
		else
		{
			m_inputMap->OnInputEventModified(shared_from_this());
		}

	}


}