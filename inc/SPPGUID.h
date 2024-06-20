// Copyright (c) David Sleeper (Sleeping Robot LLC)
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.

#pragma once

#include "SPPCore.h"

namespace SPP
{
	struct GUID
	{
		uint32_t A = 0;
		uint32_t B = 0;
		uint32_t C = 0;
		uint32_t D = 0;

		std::string ToString() const {
			return "no implementation";
		}
	};
}