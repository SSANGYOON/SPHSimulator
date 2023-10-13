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

		// �̹� ������ Ű������ �ٸ� ���ҽ��� ���� ��ϵǾ� �־���.
		if (iter != _resources.end())
		{
			return std::dynamic_pointer_cast<T>(iter->second);
		}

		return nullptr;
	}

	template <typename T>
	std::shared_ptr<T> Load(const std::wstring& key, const std::wstring& path, bool stockObject = true)
	{
		// Ű������ Ž��
		std::shared_ptr<T> resource = Resources::Find<T>(key);
		if (nullptr != resource)
		{
			// �ش�Ű�� �̹� �ε��Ȱ� ������ �ش� ���ҽ��� ��ȯ
			return resource;
		}

		// �ش� Ű�� �ε��� ���ҽ��� ����.
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