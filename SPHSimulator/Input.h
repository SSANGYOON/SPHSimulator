#pragma once

enum class KEY_TYPE : UINT8
{
	UP = VK_UP,
	DOWN = VK_DOWN,
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,
	SPACE = VK_SPACE,
	Q = 'Q', W = 'W', E = 'E', R = 'R', T = 'T', Y = 'Y', U = 'U', I = 'I', O = 'O', P = 'P',
	A = 'A', S = 'S', D = 'D', F = 'F', G = 'G', H = 'H', J = 'J', K = 'K', L = 'L',
	Z = 'Z', X = 'X', C = 'C', V = 'V', B = 'B', N = 'N', M = 'M',

	DELETEKEY = VK_DELETE,
	LALT = VK_LMENU,
	RALT = VK_RMENU,
	TAB = VK_TAB,
	LCTRL = VK_LCONTROL,
	RCTRL = VK_RCONTROL,

	LSHIFT = VK_LSHIFT,
	RSHIFT = VK_RSHIFT,

	LBUTTON = VK_LBUTTON,
	MBUTTON = VK_MBUTTON,
	RBUTTON = VK_RBUTTON
};

enum class KEY_STATE
{
	NONE,
	PRESS,
	DOWN,
	UP,
	END
};

enum
{
	KEY_TYPE_COUNT = static_cast<UINT32>(UINT8_MAX + 1),
	KEY_STATE_COUNT = static_cast<UINT32>(KEY_STATE::END),
};

class Input
{
	DECLARE_SINGLE(Input);

public:

	void Init(HWND hwnd);
	POINT GetMousePos() { return _mousePos; }
	void Update();

	KEY_STATE GetKeyState(KEY_TYPE type) { return _states[(UINT)type]; }

private:
	HWND _hwnd;
	vector<KEY_STATE> _states;
	POINT _mousePos = {};
};