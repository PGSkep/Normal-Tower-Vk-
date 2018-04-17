#ifndef CONSOLE_H
#define CONSOLE_H

#include <Windows.h>

namespace CONSOLE
{
	enum COLORS
	{
		BLACK = 0,
		BLUE = 1,
		GREEN = 2,
		CYAN = 3,
		RED = 4,
		PINK = 5,
		YELLOW = 6,
		WHITE = 7,
	};

	enum INTENSITY
	{
		LOW = 0,
		HIGH = 8
	};

	enum WINDOW_ZORDER
	{
		ZORDER_TOPMOST = ((long long)HWND_TOPMOST),
		ZORDER_NOT_TOPMOST = ((long long)HWND_NOTOPMOST),
	};

	//enum WINDOW_ACTION
	//{
	//	ACTION_NO_LAYER = SWP_NOZORDER,
	//	ACTION_NO_RESIZE = SWP_NOSIZE,
	//	ACTION_NO_MOVE = SWP_NOMOVE,
	//
	//	ACTION_HIDE = SWP_HIDEWINDOW,
	//	ACTION_SHOW = SWP_SHOWWINDOW,
	//};

	static void SetColor(short foreground, short fore_intensity, short background, short back_intensity)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (foreground + fore_intensity) + ((background + back_intensity) * 16));
	}

	static void SetPosition(int _x, int _y)
	{
		SetWindowPos(GetConsoleWindow(), 0, _x, _y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	static void SetSize(int _width, int _height)
	{
		SetWindowPos(GetConsoleWindow(), 0, 0, 0, _width, _height, SWP_NOZORDER | SWP_NOMOVE);
	}

	static void SetZOrder(WINDOW_ZORDER _layer)
	{
		SetWindowPos(GetConsoleWindow(), (HWND)_layer, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}

	static void SetVisible(bool _visible)
	{
		int visibility;

		if (_visible == true)
			visibility = SWP_SHOWWINDOW;
		else
			visibility = SWP_HIDEWINDOW;

		SetWindowPos(GetConsoleWindow(), 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | visibility);
	}
};

#endif