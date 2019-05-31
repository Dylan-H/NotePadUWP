//
//          Copyright (c) 1990-2011, Scientific Toolworks, Inc.
//
// The License.txt file describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//
// Additions Copyright (c) 2011 Archaeopteryx Software, Inc. d/b/a Wingware
// Scintilla platform layer for Qt

#ifndef PLATWINRT_H
#define PLATWINRT_H

#include <cstddef>

#include <string_view>
#include <vector>
#include <memory>

#include "Platform.h"

namespace Scintilla {

	extern void Platform_Initialise(void *hInstance);
	extern void Platform_Finalise(bool fromDllMain);


}

#endif