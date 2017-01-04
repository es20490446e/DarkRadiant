#pragma once

#include "MenuElement.h"
#include <wx/menu.h>

namespace ui
{

class MenuBar :
	public MenuElement
{
private:
	wxMenuBar* _menuBar;

public:
	virtual wxMenuBar* getWidget() override;

protected:
	virtual void constructWidget() override;
};

}

