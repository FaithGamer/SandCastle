#pragma once

//The component using this macro will give reliable pointers
//Trading off some performance when iterating over a view of it.
//Do not use component pointer without this.
#define PointableComponent static constexpr auto in_place_delete = true;