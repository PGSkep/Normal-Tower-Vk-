#ifndef	INPUT_H
#define INPUT_H

#include <array>
#include <vector>

#include <Windows.h>

#include <iostream>

class Input
{
public:
	enum INPUT_KEYS
	{
		KEY_MOUSE_LEFT = VK_LBUTTON,
		KEY_MOUSE_RIGHT = VK_RBUTTON,
		KEY_MOUSE_MIDDLE = VK_MBUTTON,
		KEY_MOUSE_X1 = VK_XBUTTON1,
		KEY_MOUSE_X2 = VK_XBUTTON2,
		KEY_Q = 'Q',
		KEY_W = 'W',
		KEY_E = 'E',
		KEY_R = 'R',
		KEY_T = 'T',
		KEY_Y = 'Y',
		KEY_U = 'U',
		KEY_I = 'I',
		KEY_O = 'O',
		KEY_P = 'P',
		KEY_A = 'A',
		KEY_S = 'S',
		KEY_D = 'D',
		KEY_F = 'F',
		KEY_G = 'G',
		KEY_H = 'H',
		KEY_J = 'J',
		KEY_K = 'K',
		KEY_L = 'L',
		KEY_Z = 'Z',
		KEY_X = 'X',
		KEY_C = 'C',
		KEY_V = 'V',
		KEY_B = 'B',
		KEY_N = 'N',
		KEY_M = 'M',
		KEY_0 = 0x30,
		KEY_1 = 0x31,
		KEY_2 = 0x32,
		KEY_3 = 0x33,
		KEY_4 = 0x34,
		KEY_5 = 0x35,
		KEY_6 = 0x36,
		KEY_7 = 0x37,
		KEY_8 = 0x38,
		KEY_9 = 0x39,
		KEY_CTRL = VK_CONTROL,
		KEY_CTRL_L = VK_LCONTROL,
		KEY_CTRL_R = VK_RCONTROL,
		KEY_SHIFT = VK_SHIFT,
		KEY_SHIFT_L = VK_LSHIFT,
		KEY_SHIFT_R = VK_RSHIFT,
		KEY_ALT = VK_MENU,
		KEY_ALT_L = VK_LMENU,
		KEY_ALT_R = VK_RMENU,
		KEY_TAB = VK_TAB,
		KEY_CAPS = VK_CAPITAL,
		KEY_RETURN = VK_RETURN,
		KEY_SPACE = VK_SPACE,
		KEY_ESC = VK_ESCAPE,
		KEY_DELETE = VK_DELETE,
		KEY_BACKSPACE = VK_BACK,
		KEY_ARROW_UP = VK_UP,
		KEY_ARROW_DOWN = VK_DOWN,
		KEY_ARROW_LEFT = VK_LEFT,
		KEY_ARROW_RIGHT = VK_RIGHT,
		KEY_PAD_0 = VK_NUMPAD0,
		KEY_PAD_1 = VK_NUMPAD1,
		KEY_PAD_2 = VK_NUMPAD2,
		KEY_PAD_3 = VK_NUMPAD3,
		KEY_PAD_4 = VK_NUMPAD4,
		KEY_PAD_5 = VK_NUMPAD5,
		KEY_PAD_6 = VK_NUMPAD6,
		KEY_PAD_7 = VK_NUMPAD7,
		KEY_PAD_8 = VK_NUMPAD8,
		KEY_PAD_9 = VK_NUMPAD9,
		KEY_PAD_MULT = VK_MULTIPLY,
		KEY_PAD_ADD = VK_ADD,
		KEY_PAD_SUBTRACT = VK_SUBTRACT,
		KEY_PAD_DIVIDE = VK_DIVIDE,
		KEY_F1 = VK_F1,
		KEY_F2 = VK_F2,
		KEY_F3 = VK_F3,
		KEY_F4 = VK_F4,
		KEY_F5 = VK_F5,
		KEY_F6 = VK_F6,
		KEY_F7 = VK_F7,
		KEY_F8 = VK_F8,
		KEY_F9 = VK_F9,
		KEY_F10 = VK_F10,
		KEY_F11 = VK_F11,
		KEY_F12 = VK_F12,
		KEY_F13 = VK_F13,
		KEY_F14 = VK_F14,
		KEY_F15 = VK_F15,
		KEY_F16 = VK_F16,
		KEY_F17 = VK_F17,
		KEY_F18 = VK_F18,
		KEY_F19 = VK_F19,
		KEY_F20 = VK_F20,
		KEY_F21 = VK_F21,
		KEY_F22 = VK_F22,
		KEY_F23 = VK_F23,
		KEY_F24 = VK_F24,
		KEY_SEMICOLON = VK_OEM_1,
		KEY_PLUS = VK_OEM_PLUS,
		KEY_MINUS = VK_OEM_MINUS,
		KEY_DOT = VK_OEM_PERIOD,
		KEY_COMMA = VK_OEM_COMMA,
		KEY_SLASH_BACK = VK_OEM_2,
		KEY_SLASH_FORWARD = VK_OEM_5,
		KEY_TILDE = VK_OEM_3,
		KEY_BRACKET_OPEN = VK_OEM_4,
		KEY_BRACKET_CLOSE = VK_OEM_6,
		KEY_QUOTE = VK_OEM_7,
	};

	enum INPUT_ATTRIBUTE
	{
		MOUSE_X = 0,
		MOUSE_Y = 1
	};

	POINT cursorLastPosition;
	POINT cursorCurrentPosition;
	POINT cursorDelta;
	
	struct KeyData
	{
		bool prevPressState = false;
		bool pressState = false;
	};
	std::array<KeyData, 256> keyboardKeys;
	std::vector<INPUT_KEYS> activeKeys;

	/*
	struct InputState
	{
	enum KeyStates
	{
	IDLE,
	HOLD,
	PRESS,
	DOWN,
	};

	struct KeyState
	{
	INPUT_KEYS key;
	KeyStates state;
	float stateAccumulatedTime;
	};
	std::vector<KeyState> activeKeys;
	float time;
	std::array<long, 2> mousePosition;
	};
	std::vector<InputState> inputStateQueue;
	//*/

	void ActivateKey(INPUT_KEYS _key)
	{
		activeKeys.push_back(_key);
	}
	void ActivateAllKeys()
	{
		for (uint8_t i = 0; i != 255; ++i)
			activeKeys.push_back((INPUT_KEYS)i);
	}
	void ClearKey(INPUT_KEYS _key)
	{
		activeKeys.clear();
	}
	void Update()
	{
		cursorLastPosition = cursorCurrentPosition;
		GetCursorPos(&cursorCurrentPosition);

		cursorDelta.x = cursorCurrentPosition.x - cursorLastPosition.x;
		cursorDelta.y = cursorCurrentPosition.y - cursorLastPosition.y;

		for (size_t i = 0; i != activeKeys.size(); ++i)
		{
			keyboardKeys[(uint8_t)activeKeys[i]].prevPressState = keyboardKeys[activeKeys[i]].pressState;
			keyboardKeys[(uint8_t)activeKeys[i]].pressState = GetAsyncKeyState(activeKeys[i]) != 0;
		}
	}

	bool CheckKeyUp(INPUT_KEYS _key)
	{
		if (keyboardKeys[_key].pressState == false)
			return true;
		return false;
	}
	bool CheckKeyDown(INPUT_KEYS _key)
	{
		if (keyboardKeys[_key].pressState == true)
			return true;
		return false;
	}
	bool CheckKeyHold(INPUT_KEYS _key)
	{
		if (keyboardKeys[_key].prevPressState == true && keyboardKeys[_key].pressState == true)
			return true;
		return false;
	}
	bool CheckKeyIdle(INPUT_KEYS _key)
	{
		if (keyboardKeys[_key].prevPressState == false && keyboardKeys[_key].pressState == false)
			return true;
		return false;
	}
	bool CheckKeyPress(INPUT_KEYS _key)
	{
		if (keyboardKeys[_key].prevPressState == false && keyboardKeys[_key].pressState == true)
			return true;
		return false;
	}
	bool CheckKeyRelease(INPUT_KEYS _key)
	{
		if (keyboardKeys[_key].prevPressState == true && keyboardKeys[_key].pressState == false)
			return true;
		return false;
	}

	/*
	std::string text;
	void Test()
	{
	for (size_t i = 0; i != activeKeys.size(); ++i)
	{
	if (CheckKeyPress(activeKeys[i]))
	text += activeKeys[i];
	}

	system("cls");
	std::cout << "Mouse X = " << mousePosition[0] << "\nMouse Y = " << mousePosition[1] << '\n' << text;
	}
	//*/
};

#endif