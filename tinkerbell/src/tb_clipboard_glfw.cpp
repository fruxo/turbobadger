// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_system.h"
#include "GL/glfw3.h"

namespace tinkerbell {

// == TBClipboard =====================================

void TBClipboard::Empty()
{
	SetText("");
}

bool TBClipboard::HasText()
{
	if (GLFWwindow window = glfwGetCurrentContext())
	{
		const char *str = glfwGetClipboardString(window);
		if (str && *str)
			return true;
	}
	return false;
}

bool TBClipboard::SetText(const char *text)
{
	if (GLFWwindow window = glfwGetCurrentContext())
	{
		glfwSetClipboardString(window, text);
		return true;
	}
	return false;
}

bool TBClipboard::GetText(TBStr &text)
{
	if (GLFWwindow window = glfwGetCurrentContext())
	{
		if (const char *str = glfwGetClipboardString(window))
			return text.Set(str);
	}
	return false;
}

}; // namespace tinkerbell
