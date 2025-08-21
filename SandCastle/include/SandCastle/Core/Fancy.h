#pragma once

#ifdef SC_FANCY
#define sys(system) Systems::Get<system>()
#endif