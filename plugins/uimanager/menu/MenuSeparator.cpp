#include "MenuSeparator.h"

#include "itextstream.h"
#include <wx/menu.h>

#include "MenuFolder.h"

namespace ui
{

wxMenuItem* MenuSeparator::getWidget()
{
	if (_separator == nullptr)
	{
		constructWidget();
	}

	return _separator;
}

void MenuSeparator::constructWidget()
{
	if (_separator != nullptr)
	{
		MenuElement::constructWidget();
		return;
	}

	// Get the parent menu
	MenuElementPtr parent = getParent();

	if (!parent || !std::dynamic_pointer_cast<MenuFolder>(parent))
	{
		rWarning() << "Cannot construct separator without a parent menu" << std::endl;
		return;
	}

	wxMenu* menu = std::static_pointer_cast<MenuFolder>(parent)->getWidget();

	_separator = menu->AppendSeparator();

	MenuElement::constructWidget();
}

}
