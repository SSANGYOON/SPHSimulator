#pragma once
#include "Resource.h"

class Resources
{
	DECLARE_SINGLE(Resources);

public:

	void CreateDefaultResource();
	template <typename T>
	shared_ptr<T> Find(const std::wstring& key)
	{
		std::map<std::wstring, std::shared_ptr<Resource>>::iterator iter = _resources.find(key);

		// 이미 동일한 키값으로 다른 리소스가 먼저 등록되어 있었다.
		if (iter != _resources.end())
		{
			return std::dynamic_pointer_cast<T>(iter->second);
		}

		return nullptr;
	}

	template <typename T>
	std::shared_ptr<T> Load(const std::wstring& key, const std::wstring& path, bool stockObject = true)
	{
		// 키값으로 탐색
		std::shared_ptr<T> resource = Resources::Find<T>(key);
		if (nullptr != resource)
		{
			// 해당키로 이미 로딩된게 있으면 해당 리소스를 반환
			return resource;
		}

		// 해당 키로 로딩된 리소스가 없다.
		resource = std::make_shared<T>();
		if (FAILED(resource->Load(path, stockObject)))
		{
			MessageBox(nullptr, L"Image Load Failed!", L"Error", MB_OK);
			return nullptr;
		}

		resource->SetKey(key);
		_resources.insert(std::make_pair(key, static_pointer_cast<Resource>(resource)));

		return resource;
	}

	template <typename T>
	void Insert(const wstring& key, shared_ptr<T> resource)
	{
		resource->SetKey(key);
		_resources.insert(make_pair(key, dynamic_pointer_cast<Resource>(resource)));
	}


private:
	map<wstring, shared_ptr<Resource>> _resources;
};