#pragma once

/// @file Input.h
/// @brief Aggregate include for the Input module.
/// User code creates an InputMap, adds named ButtonInput / DirectionalInput
/// entries with one or more bindings (Keys, Mouse, Gamepad), and listens to
/// each Input's signal to react. Inputs is the global registry; Bindings.h
/// describes the data model used by SetBindings.

#include "SandCastle/Input/Input.h"
#include "SandCastle/Input/Bindings.h"
#include "SandCastle/Input/ButtonInput.h"
#include "SandCastle/Input/Gamepad.h"
#include "SandCastle/Input/DirectionalInput.h"
#include "SandCastle/Input/InputMap.h"
#include "SandCastle/Input/Inputs.h"
#include "SandCastle/Input/Keyboard.h"
#include "SandCastle/Input/Mouse.h"
#include "SandCastle/Input/TextualInput.h"