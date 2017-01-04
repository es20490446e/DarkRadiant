#pragma once

#include "xmlutil/Node.h"
#include "iuimanager.h"
#include <vector>
#include <memory>

/** greebo: This is a representation of a general menu item/element.
 *
 * The possible menuitem types are defined in iuimanager.h.
 * Each menu item can have a list of sub-menuitems (this applies to the
 * types eMenuBar and eFolder).
 *
 * Use the MenuManager class to access these menuitems.
 */
namespace ui
{

class MenuItem;
typedef std::shared_ptr<MenuItem> MenuItemPtr;
typedef std::weak_ptr<MenuItem> MenuItemWeakPtr;

class MenuItem :
	public std::enable_shared_from_this<MenuItem>
{
	// The parent of this MenuItem (weak reference to avoid circular ownership)
	MenuItemWeakPtr _parent;

	// The name of this node
	std::string _name;

	// The caption (display string) incl. the mnemonic
	std::string _caption;

	// The icon name
	std::string _icon;

	// The associated event
	std::string _event;

	wxObject* _widget;

	// The children of this MenuItem
	typedef std::vector<MenuItemPtr> MenuItemList;
	MenuItemList _children;

	eMenuItemType _type;

	// Stays false until the widgets are actually created.
	bool _constructed;

	static int _nextMenuItemId;

public:
	// Constructor, needs a name and a parent specified
	MenuItem(const MenuItemPtr& parent = MenuItemPtr());

	// Destructor disconnects the widget from the event
	~MenuItem();

	// The name of this MenuItem
	std::string getName() const;
	void setName(const std::string& name);

	void setIcon(const std::string& icon);

	// Returns TRUE if this item has no parent item
	bool isRoot() const;

	// Returns the pointer to the parent (is NULL for the root item)
	MenuItemPtr parent() const;
	void setParent(const MenuItemPtr& parent);

	/** greebo: Adds the given menuitem to the list of children.
	 *
	 *  Note: the new child is NOT reparented, the calling function must to this.
	 */
	void addChild(const MenuItemPtr& newChild);

	/**
	 * Removes the given child from this menu item.
	 */
	void removeChild(const MenuItemPtr& child);

	// Removes all child nodes
	void removeAllChildren();

	/** greebo: Tries to find the GtkMenu position index of the given child.
	 */
	int getMenuPosition(const MenuItemPtr& child);

	// Returns the type of this item node
	eMenuItemType getType() const;
	void setType(eMenuItemType type);

	// Gets/sets the caption of this item
	void setCaption(const std::string& caption);
	std::string getCaption() const;

	// Returns TRUE if this has no actual event assigned
	bool isEmpty() const;

	// Returns the number of child items
	std::size_t numChildren() const;

	// Return / set the event name
	std::string getEvent() const;
	void setEvent(const std::string& eventName);

	void connectEvent();
	void disconnectEvent();

	// Use this to get the corresponding wx menu widget out of this item.
	wxObject* getWidget();
	void setWidget(wxObject* object);

	// Tries to (recursively) locate the menuitem by looking up the path
	MenuItemPtr find(const std::string& menuPath);

	/**
	 * Parses the given XML node recursively and creates all items from the 
	 * information it finds. Returns the constructed MenuItem.
	 */
	static MenuItemPtr CreateFromNode(const xml::Node& node);

private:
	/** greebo: This constructs the actual widgets. This is invoked as soon
	 * 			as the first getWidget of this object is requested.
	 */
	void construct();

	static eMenuItemType GetTypeForXmlNode(const xml::Node& node);
};

} // namespace ui
