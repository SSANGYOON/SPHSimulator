#pragma once

enum class RESOURCE_TYPE
{
	MESH,
	TEXTURE,
	GRAPHIC_SHADER,
	COMPUTE_SHADER,
};

class Resource
{
public:
	Resource(RESOURCE_TYPE type);
	virtual ~Resource();

	virtual HRESULT Load(const std::wstring& path, bool stockObject = true) = 0;

	void SetKey(const wstring& key) { _key = key; }
	void SetPath(const wstring& path) { _path = path; }

	UINT GetId() { return _id; }
	const wstring& GetKey() { return _key; }
	const wstring& GetPath() { return _path; }

private:
	friend class Resources;
	RESOURCE_TYPE _type;

	UINT _id = 0;
	wstring _key;
	wstring _path;
};
