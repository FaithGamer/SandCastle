#pragma once

/// @file UIheader.h
/// @brief Aggregate include for the UI module: an immediate-mode-ish layout
/// system built on the engine's renderer/ECS. Build a tree with
/// Ui::Begin/End and the Text/Button/Image/Checkbox/LoadBar helpers; styling
/// flows through a stack of contexts (font, colors, frames, padding...).

#include "SandCastle/UI/Ui.h"
#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiCanvas.h"
#include "SandCastle/UI/UiTxt.h"
#include "SandCastle/UI/UiImg.h"
#include "SandCastle/UI/UiBtn.h"
#include "SandCastle/UI/UiAnimBtn.h"
#include "SandCastle/UI/UiFrame.h"
#include "SandCastle/UI/UiCheckbox.h"
#include "SandCastle/UI/UiLoadBar.h"
#include "SandCastle/UI/UiEnum.h"
