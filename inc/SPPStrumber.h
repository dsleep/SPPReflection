// Copyright (c) David Sleeper (Sleeping Robot LLC)
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.

#pragma once

#include "SPPCore.h"
#include <vector>
#include <list>
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <unordered_map>

namespace SPP
{
	static constexpr uint16_t const STRUMBER_NO_NUMBER = 0xFFFF;

	struct Strumber
	{
		uint32_t _id = 0;
		uint16_t _number = STRUMBER_NO_NUMBER;

		std::string ToString() const {
			return "no implementation";
		}
	};			
}