#pragma once

/// @brief Tag a component type as "pointable": components stay at a stable
/// address (in_place_delete = true) so raw pointers/references survive
/// insertions and deletions, at the cost of slightly slower iteration.
/// Required for components whose pointer you intend to cache (e.g. Camera).
#define PointableComponent static constexpr auto in_place_delete = true;