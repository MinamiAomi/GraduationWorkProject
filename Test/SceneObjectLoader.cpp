#include "SceneObjectLoader.h"

#include <iostream>

#include "File/JsonConverter.h"
#include "SceneObjectData.h"

#ifdef _DEBUG
#include <iomanip>
#include <sstream>
#include <Windows.h>
#endif // _DEBUG

namespace SceneObjectSystem {
	std::vector<SceneObjectSystem::SceneObjectData> SceneObjectSystem::SceneLoader::LoadSceneFromFile(const std::filesystem::path& filePath)
	{
		std::ifstream ifs(filePath);

		if (!ifs.is_open()) {
#ifdef _DEBUG
			{
				std::wostringstream woss;
				woss << L"ファイルが開けません:" << filePath << L"\n";
				OutputDebugStringW(woss.str().c_str());
			}
#endif // _DEBUG
			return {};
		}

		nlohmann::json j;

		try {
			j = nlohmann::json::parse(ifs);
			const nlohmann::json& objectJson = j.at("objects");

			return objectJson.get<std::vector<SceneObjectSystem::SceneObjectData>>();
		}
		catch (nlohmann::json::exception& e) {
			e;
#ifdef _DEBUG
			{
				std::wostringstream woss;
				woss << L"パースに失敗:" << filePath << L"\n"
					<< "File: " << filePath << L"\n"
					<< "Error: " << e.what() << L"\n";
				OutputDebugStringW(woss.str().c_str());
			}
#endif // _DEBUG
			return {};
		}
	}
}
