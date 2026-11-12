#include "Asset.h"

#include <utility>

Asset::Asset(std::string newName, std::string newPath)
	: name(std::move(newName)), path(std::move(newPath))
{
}

const std::string& Asset::GetName() const
{
	return name;
}

const std::string& Asset::GetPath() const
{
	return path;
}
