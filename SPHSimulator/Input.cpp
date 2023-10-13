#include "pch.h"
#include "Input.h"

void Input::Init(HWND hwnd)
{
	_hwnd = hwnd;
	_states.resize(UINT8_MAX + 1, KEY_STATE::NONE);
}

void Input::Update()
{
	HWND hwnd = ::GetActiveWindow();
	if (_hwnd != hwnd)
	{
		for (auto& state : _states)
			state = KEY_STATE::NONE;
	}
	else
	{
		BYTE asciiKeys[KEY_TYPE_COUNT] = {};
		if (::GetKeyboardState(asciiKeys) == false)
			return;

		for (UINT32 key = 0; key < KEY_TYPE_COUNT; key++)
		{
			// 키가 눌려 있으면 true
			if (asciiKeys[key] & 0x80)
			{
				KEY_STATE& state = _states[key];

				// 이전 프레임에 키를 누른 상태라면 PRESS
				if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
					state = KEY_STATE::PRESS;
				else
					state = KEY_STATE::DOWN;
			}
			else
			{
				KEY_STATE& state = _states[key];

				// 이전 프레임에 키를 누른 상태라면 UP
				if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
					state = KEY_STATE::UP;
				else
					state = KEY_STATE::NONE;
			}
		}
		::GetCursorPos(&_mousePos);
		::ScreenToClient(_hwnd, &_mousePos);
	}
}