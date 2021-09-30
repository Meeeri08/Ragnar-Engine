#pragma once

#include "Imgui/imgui.h"

class ConsoleMenu
{
public:
	ConsoleMenu();

	~ConsoleMenu();

	bool Update();

	void AddLog(const char* fmt);

	inline bool& GetActive() { return console; }

private:
	ImGuiTextBuffer buf;
	bool scrollToBottom;

	bool console;
};