#pragma once

#include <filesystem>

#include "common.h"

#ifdef API_EXPORTS
#define API_EXPORTS __declspec(dllexport)
#else
#define API_EXPORTS __declspec(dllimport)
#endif

extern "C"
{
	namespace Testlab
	{
		API_EXPORTS IProject* createProject();
		API_EXPORTS IProject* openProject(std::filesystem::path const& pathFile);
	}
}