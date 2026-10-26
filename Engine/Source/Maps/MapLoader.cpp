#include "MapLoader.h"

#include "Actor.h"
#include "Components/ActorComponent.h"
#include "Core/Containers/Array.h"
#include "Core/Containers/SeonString.h"
#include "Core/Logging/Log.h"
#include "Core/Templates/UniquePtr.h"
#include "Maps/ActorTypeRegistry.h"
#include "Maps/ComponentTypeRegistry.h"
#include "Properties/ComponentPropertyInfo.h"
#include "Properties/PropertyValue.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <variant>

namespace
{
	const FComponentPropertyInfo* FindProperty(std::span<const FComponentPropertyInfo> properties, FStringView name)
	{
		for (const FComponentPropertyInfo& property : properties)
		{
			if (property.name == name) { return &property; }
		}

		return nullptr;
	}

	bool ValidateComponentProperties(const rapidjson::Value& data, std::span<const FComponentPropertyInfo> properties, FStringView pathText, rapidjson::SizeType actorIndex, rapidjson::SizeType componentIndex)
	{
		TArray<FStringView> names;
		for (const auto& member : data.GetObject())
		{
			const FStringView name(member.name.GetString(), member.name.GetStringLength());
			if (name == "name" || name == "type") { continue; }

			const FComponentPropertyInfo* property = FindProperty(properties, name);
			if (!property || !property->getValue || !property->setValue)
			{
				WriteLog("Map", ELogLevel::Error, "Property '{}' of component at index {} of actor at index {} in JSON map file '{}' must be registered with read and write accessors.", name, componentIndex, actorIndex, pathText);
				return false;
			}

			if (std::find(names.cbegin(), names.cend(), name) != names.cend())
			{
				WriteLog("Map", ELogLevel::Error, "Duplicate property '{}' of component at index {} of actor at index {} in JSON map file '{}'.", name, componentIndex, actorIndex, pathText);
				return false;
			}
			names.push_back(name);

			if (!member.value.IsArray() || member.value.Size() != 3)
			{
				WriteLog("Map", ELogLevel::Error, "Property '{}' of component at index {} of actor at index {} in JSON map file '{}' must be an array of three numbers.", name, componentIndex, actorIndex, pathText);
				return false;
			}

			for (const rapidjson::Value& element : member.value.GetArray())
			{
				if (!element.IsNumber() || !std::isfinite(element.GetDouble()) || std::abs(element.GetDouble()) > std::numeric_limits<float>::max())
				{
					WriteLog("Map", ELogLevel::Error, "Property '{}' of component at index {} of actor at index {} in JSON map file '{}' must contain finite numbers within the float range.", name, componentIndex, actorIndex, pathText);
					return false;
				}
			}
		}

		return true;
	}

	bool ApplyComponentProperties(const rapidjson::Value& data, UActorComponent& component, std::span<const FComponentPropertyInfo> properties, FStringView pathText, rapidjson::SizeType actorIndex, rapidjson::SizeType componentIndex)
	{
		for (const auto& member : data.GetObject())
		{
			const FStringView name(member.name.GetString(), member.name.GetStringLength());
			if (name == "name" || name == "type") { continue; }

			const FComponentPropertyInfo* property = FindProperty(properties, name);
			FPropertyValue value;
			if (!property || !property->getValue || !property->setValue || !property->getValue(component, value))
			{
				WriteLog("Map", ELogLevel::Error, "Reading property '{}' of component at index {} of actor at index {} in JSON map file '{}' failed.", name, componentIndex, actorIndex, pathText);
				return false;
			}

			// 파일 전체 검사로 세 숫자의 범위를 확인했다. getter가 선택한 값 타입에 맞춰 변환한다.
			const float first = static_cast<float>(member.value[0].GetDouble());
			const float second = static_cast<float>(member.value[1].GetDouble());
			const float third = static_cast<float>(member.value[2].GetDouble());
			std::visit([first, second, third](auto& typedValue)
			{
				typedValue = {first, second, third};
			}, value);
			if (!property->setValue(component, value))
			{
				WriteLog("Map", ELogLevel::Error, "Applying property '{}' of component at index {} of actor at index {} in JSON map file '{}' failed.", name, componentIndex, actorIndex, pathText);
				return false;
			}
		}

		return true;
	}

	bool LoadActorComponents(const rapidjson::Value& components, AActor& actor, const FComponentTypeRegistry& componentTypes, FStringView pathText, rapidjson::SizeType actorIndex)
	{
		for (rapidjson::SizeType index = 0; index < components.Size(); ++index)
		{
			const rapidjson::Value& data = components[index];
			const rapidjson::Value& name = data["name"];
			const rapidjson::Value& type = data["type"];
			const FStringView componentName(name.GetString(), name.GetStringLength());
			const FStringView typeName(type.GetString(), type.GetStringLength());

			if (UActorComponent* existing = actor.FindComponent(componentName))
			{
				if (!componentTypes.IsType(typeName, *existing))
				{
					WriteLog("Map", ELogLevel::Error, "Component '{}' at index {} of actor at index {} in JSON map file '{}' does not match registered type '{}'.", componentName, index, actorIndex, pathText, typeName);
					return false;
				}
				if (!ApplyComponentProperties(data, *existing, componentTypes.GetProperties(typeName), pathText, actorIndex, index)) { return false; }
				continue;
			}

			const FComponentFactory factory = componentTypes.Find(typeName);
			if (!factory)
			{
				WriteLog("Map", ELogLevel::Error, "Component at index {} of actor at index {} in JSON map file '{}' has no factory for type '{}'.", index, actorIndex, pathText, typeName);
				return false;
			}

			// 추가하지 못한 후보는 이 소유 포인터가 정리하며 기존 컴포넌트에는 손대지 않는다.
			TUniquePtr<UActorComponent> component = factory(actor);
			if (!component)
			{
				WriteLog("Map", ELogLevel::Error, "Factory for component type '{}' at index {} of actor at index {} in JSON map file '{}' returned null.", typeName, index, actorIndex, pathText);
				return false;
			}
			if (!componentTypes.IsType(typeName, *component))
			{
				WriteLog("Map", ELogLevel::Error, "Factory for component type '{}' at index {} of actor at index {} in JSON map file '{}' returned the wrong type.", typeName, index, actorIndex, pathText);
				return false;
			}
			if (&component->GetOwner() != &actor)
			{
				WriteLog("Map", ELogLevel::Error, "Factory for component type '{}' at index {} of actor at index {} in JSON map file '{}' returned a component with another owner.", typeName, index, actorIndex, pathText);
				return false;
			}
			if (!ApplyComponentProperties(data, *component, componentTypes.GetProperties(typeName), pathText, actorIndex, index)) { return false; }

			if (!actor.AddComponent(componentName, component))
			{
				WriteLog("Map", ELogLevel::Error, "Adding component '{}' at index {} of actor at index {} in JSON map file '{}' failed.", componentName, index, actorIndex, pathText);
				return false;
			}
		}

		return true;
	}
}

bool FMapLoader::LoadMapFile(const std::filesystem::path& path, const FActorTypeRegistry& actorTypes, const FComponentTypeRegistry& componentTypes, UWorld& world)
{
	// 경로 로그는 UTF-8로 출력하며 빌린 뷰는 아래 소유 문자열의 수명 안에서만 사용한다.
	const std::u8string pathUtf8 = path.u8string();
	const FStringView pathText(reinterpret_cast<const char*>(pathUtf8.data()), pathUtf8.size());
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		WriteLog("Map", ELogLevel::Error, "Opening JSON file '{}' failed.", pathText);
		return false;
	}

	const std::streamoff fileSize = file.tellg();
	if (fileSize < 0)
	{
		WriteLog("Map", ELogLevel::Error, "Reading the size of JSON file '{}' failed.", pathText);
		return false;
	}

	FString jsonText;
	// 파일 크기를 문자열 길이와 읽기 크기로 변환하기 전에 표현 범위를 확인한다.
	if (std::cmp_greater(fileSize, jsonText.max_size()) || std::cmp_greater(fileSize, std::numeric_limits<std::streamsize>::max()))
	{
		WriteLog("Map", ELogLevel::Error, "JSON file '{}' is too large for the text buffer or a stream read.", pathText);
		return false;
	}

	jsonText.resize(static_cast<FString::size_type>(fileSize));

	file.seekg(0, std::ios::beg);
	if (!file)
	{
		WriteLog("Map", ELogLevel::Error, "Seeking to the beginning of JSON file '{}' failed.", pathText);
		return false;
	}

	if (!file.read(jsonText.data(), static_cast<std::streamsize>(fileSize)))
	{
		WriteLog("Map", ELogLevel::Error, "Reading JSON file '{}' failed.", pathText);
		return false;
	}

	// 파서가 실제 NUL을 입력 끝으로 해석해 뒤의 내용을 놓치는 것을 방지한다.
	const FString::size_type nullOffset = jsonText.find('\0');
	if (nullOffset != FString::npos)
	{
		WriteLog("Map", ELogLevel::Error, "JSON file '{}' contains a NUL byte at offset {}.", pathText, nullOffset);
		return false;
	}

	rapidjson::Document document;
	document.Parse<rapidjson::kParseValidateEncodingFlag>(jsonText.data(), jsonText.size());
	if (document.HasParseError())
	{
		WriteLog("Map", ELogLevel::Error, "Parsing JSON file '{}' failed at byte offset {}: {}", pathText, document.GetErrorOffset(), rapidjson::GetParseError_En(document.GetParseError()));
		return false;
	}

	if (!document.IsObject())
	{
		WriteLog("Map", ELogLevel::Error, "JSON map file '{}' must have an object at the root.", pathText);
		return false;
	}

	// 문서 내부 값을 빌려 검사하며, 전체 검사가 끝날 때까지 실행 객체를 생성하지 않는다.
	const rapidjson::Value::ConstMemberIterator actors = document.FindMember("actors");
	if (actors == document.MemberEnd())
	{
		WriteLog("Map", ELogLevel::Error, "JSON map file '{}' is missing 'actors'.", pathText);
		return false;
	}

	if (!actors->value.IsArray())
	{
		WriteLog("Map", ELogLevel::Error, "'actors' in JSON map file '{}' must be an array.", pathText);
		return false;
	}

	TArray<FActorFactory> factories;
	for (rapidjson::SizeType index = 0; index < actors->value.Size(); ++index)
	{
		const rapidjson::Value& actorData = actors->value[index];
		if (!actorData.IsObject())
		{
			WriteLog("Map", ELogLevel::Error, "Actor at index {} in JSON map file '{}' must be an object.", index, pathText);
			return false;
		}

		const rapidjson::Value::ConstMemberIterator type = actorData.FindMember("type");
		if (type == actorData.MemberEnd())
		{
			WriteLog("Map", ELogLevel::Error, "Actor at index {} in JSON map file '{}' is missing 'type'.", index, pathText);
			return false;
		}

		if (!type->value.IsString())
		{
			WriteLog("Map", ELogLevel::Error, "'type' of actor at index {} in JSON map file '{}' must be a string.", index, pathText);
			return false;
		}

		if (type->value.GetStringLength() == 0)
		{
			WriteLog("Map", ELogLevel::Error, "'type' of actor at index {} in JSON map file '{}' must not be empty.", index, pathText);
			return false;
		}

		const FStringView actorTypeName(type->value.GetString(), type->value.GetStringLength());
		const FActorFactory factory = actorTypes.Find(actorTypeName);
		if (!factory)
		{
			WriteLog("Map", ELogLevel::Error, "Actor at index {} in JSON map file '{}' has unregistered type '{}'.", index, pathText, actorTypeName);
			return false;
		}

		const rapidjson::Value::ConstMemberIterator components = actorData.FindMember("components");
		if (components == actorData.MemberEnd())
		{
			WriteLog("Map", ELogLevel::Error, "Actor at index {} in JSON map file '{}' is missing 'components'.", index, pathText);
			return false;
		}

		if (!components->value.IsArray())
		{
			WriteLog("Map", ELogLevel::Error, "'components' of actor at index {} in JSON map file '{}' must be an array.", index, pathText);
			return false;
		}

		TArray<FStringView> componentNames;
		for (rapidjson::SizeType componentIndex = 0; componentIndex < components->value.Size(); ++componentIndex)
		{
			const rapidjson::Value& componentData = components->value[componentIndex];
			if (!componentData.IsObject())
			{
				WriteLog("Map", ELogLevel::Error, "Component at index {} of actor at index {} in JSON map file '{}' must be an object.", componentIndex, index, pathText);
				return false;
			}

			const rapidjson::Value::ConstMemberIterator name = componentData.FindMember("name");
			if (name == componentData.MemberEnd())
			{
				WriteLog("Map", ELogLevel::Error, "Component at index {} of actor at index {} in JSON map file '{}' is missing 'name'.", componentIndex, index, pathText);
				return false;
			}

			if (!name->value.IsString())
			{
				WriteLog("Map", ELogLevel::Error, "'name' of component at index {} of actor at index {} in JSON map file '{}' must be a string.", componentIndex, index, pathText);
				return false;
			}

			if (name->value.GetStringLength() == 0)
			{
				WriteLog("Map", ELogLevel::Error, "'name' of component at index {} of actor at index {} in JSON map file '{}' must not be empty.", componentIndex, index, pathText);
				return false;
			}

			const rapidjson::Value::ConstMemberIterator componentType = componentData.FindMember("type");
			if (componentType == componentData.MemberEnd())
			{
				WriteLog("Map", ELogLevel::Error, "Component at index {} of actor at index {} in JSON map file '{}' is missing 'type'.", componentIndex, index, pathText);
				return false;
			}

			if (!componentType->value.IsString())
			{
				WriteLog("Map", ELogLevel::Error, "'type' of component at index {} of actor at index {} in JSON map file '{}' must be a string.", componentIndex, index, pathText);
				return false;
			}

			if (componentType->value.GetStringLength() == 0)
			{
				WriteLog("Map", ELogLevel::Error, "'type' of component at index {} of actor at index {} in JSON map file '{}' must not be empty.", componentIndex, index, pathText);
				return false;
			}

			const FStringView componentTypeName(componentType->value.GetString(), componentType->value.GetStringLength());
			if (!componentTypes.Find(componentTypeName))
			{
				WriteLog("Map", ELogLevel::Error, "Component at index {} of actor at index {} in JSON map file '{}' has unregistered type '{}'.", componentIndex, index, pathText, componentTypeName);
				return false;
			}

			const FStringView componentName(name->value.GetString(), name->value.GetStringLength());
			if (std::find(componentNames.cbegin(), componentNames.cend(), componentName) != componentNames.cend())
			{
				WriteLog("Map", ELogLevel::Error, "Component at index {} of actor at index {} in JSON map file '{}' has duplicate name '{}'.", componentIndex, index, pathText, componentName);
				return false;
			}
			if (!ValidateComponentProperties(componentData, componentTypes.GetProperties(componentTypeName), pathText, index, componentIndex)) { return false; }
			componentNames.push_back(componentName);
		}
		factories.push_back(factory);
	}

	// 뒤쪽 항목의 검사 실패로 앞쪽 액터만 생성되는 일을 막는다.
	for (rapidjson::SizeType index = 0; index < actors->value.Size(); ++index)
	{
		AActor& actor = factories[index](world);
		const rapidjson::Value& components = actors->value[index]["components"];
		if (!LoadActorComponents(components, actor, componentTypes, pathText, index)) { return false; }
	}

	return true;
}
