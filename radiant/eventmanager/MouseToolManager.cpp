#include "MouseToolManager.h"

#include "iradiant.h"
#include "istatusbarmanager.h"
#include "iregistry.h"
#include "itextstream.h"
#include "imainframe.h"

#include "string/convert.h"
#include "string/join.h"
#include "wxutil/MouseButton.h"
#include "wxutil/Modifier.h"
#include "module/StaticModule.h"

namespace ui
{

namespace
{
    constexpr const char* const STATUS_BAR_ELEMENT = "Command";
}

MouseToolManager::MouseToolManager() :
    _activeModifierState(0),
    _hintCloseTimer(this)
{
    Bind(wxEVT_TIMER, &MouseToolManager::onCloseTimerIntervalReached, this);
}

// RegisterableModule implementation
const std::string& MouseToolManager::getName() const
{
    static std::string _name(MODULE_MOUSETOOLMANAGER);
    return _name;
}

const StringSet& MouseToolManager::getDependencies() const
{
    static StringSet _dependencies;

    if (_dependencies.empty())
    {
        _dependencies.insert(MODULE_MAINFRAME);
        _dependencies.insert(MODULE_STATUSBARMANAGER);
    }

    return _dependencies;
}

void MouseToolManager::initialiseModule(const IApplicationContext& ctx)
{
    GlobalMainFrame().signal_MainFrameConstructed().connect(
        sigc::mem_fun(this, &MouseToolManager::onMainFrameConstructed));

    // Add the statusbar command text item
    GlobalStatusBarManager().addTextElement(
        STATUS_BAR_ELEMENT,
        "",  // no icon
        statusbar::StandardPosition::Command,
        _("Describes available Mouse Commands")
    );
}

void MouseToolManager::loadGroupMapping(MouseToolGroup::Type type, const xml::NodeList& userMappings, const xml::NodeList& defaultMappings)
{
	MouseToolGroup& group = getGroup(type);

	group.clearToolMappings();

	group.foreachMouseTool([&] (const MouseToolPtr& tool)
	{
		// First, look in the userMappings if we have a user-defined setting
		for (const xml::Node& node : userMappings)
		{
			if (node.getAttributeValue("name") == tool->getName())
			{
				// Load the condition
				unsigned int state = wxutil::MouseButton::LoadFromNode(node) | wxutil::Modifier::LoadFromNode(node);
				group.addToolMapping(state, tool);

				return; // done here
			}
		}

		// nothing found in the user mapping, fall back to default
		for (const xml::Node& node : defaultMappings)
		{
			if (node.getAttributeValue("name") == tool->getName())
			{
				// Load the condition
				unsigned int state = wxutil::MouseButton::LoadFromNode(node) | wxutil::Modifier::LoadFromNode(node);
				group.addToolMapping(state, tool);

				return; // done here
			}
		}

		// No mapping for this tool
	});
}

void MouseToolManager::loadToolMappings()
{
    // All modules have registered their stuff, now load the mapping
    // Try the user-defined mapping first
    xml::NodeList userMappings = GlobalRegistry().findXPath("user/ui/input/mouseToolMappings[@name='user']//mouseToolMapping//tool");
	xml::NodeList defaultMappings = GlobalRegistry().findXPath("user/ui/input/mouseToolMappings[@name='default']//mouseToolMapping//tool");

	loadGroupMapping(MouseToolGroup::Type::CameraView, userMappings, defaultMappings);
	loadGroupMapping(MouseToolGroup::Type::OrthoView, userMappings, defaultMappings);
}

void MouseToolManager::resetBindingsToDefault()
{
    // Remove all user settings
    GlobalRegistry().deleteXPath("user/ui/input//mouseToolMappings[@name='user']");

    // Reload the bindings
    loadToolMappings();
}

void MouseToolManager::onMainFrameConstructed()
{
    loadToolMappings();
}

void MouseToolManager::saveToolMappings()
{
    GlobalRegistry().deleteXPath("user/ui/input//mouseToolMappings[@name='user']");

    xml::Node mappingsRoot = GlobalRegistry().createKeyWithName("user/ui/input", "mouseToolMappings", "user");

    foreachGroup([&] (IMouseToolGroup& g)
    {
        MouseToolGroup& group = static_cast<MouseToolGroup&>(g);
        std::string groupName = group.getType() == IMouseToolGroup::Type::OrthoView ? "OrthoView" : "CameraView";

        xml::Node mappingNode = mappingsRoot.createChild("mouseToolMapping");
        mappingNode.setAttributeValue("name", groupName);
        mappingNode.setAttributeValue("id", string::to_string(static_cast<int>(group.getType())));

        // e.g. <tool name="CameraMoveTool" button="MMB" modifiers="CONTROL" />
        group.foreachMapping([&](unsigned int state, const MouseToolPtr& tool)
        {
            xml::Node toolNode = mappingNode.createChild("tool");

            toolNode.setAttributeValue("name", tool->getName());
            wxutil::MouseButton::SaveToNode(state, toolNode);
            wxutil::Modifier::SaveToNode(state, toolNode);
        });
    });
}

void MouseToolManager::shutdownModule()
{
    // Save tool mappings
    saveToolMappings();

    _mouseToolGroups.clear();
}

MouseToolGroup& MouseToolManager::getGroup(IMouseToolGroup::Type group)
{
    GroupMap::iterator found = _mouseToolGroups.find(group);

    // Insert if not there yet
    if (found == _mouseToolGroups.end())
    {
        found = _mouseToolGroups.insert(std::make_pair(group, std::make_shared<MouseToolGroup>(group))).first;
    }

    return *found->second;
}

void MouseToolManager::foreachGroup(const std::function<void(IMouseToolGroup&)>& functor)
{
    for (auto i : _mouseToolGroups)
    {
        functor(*i.second);
    }
}

MouseToolStack MouseToolManager::getMouseToolsForEvent(IMouseToolGroup::Type group, unsigned int mouseState)
{
    return getGroup(group).getMappedTools(mouseState);
}

void MouseToolManager::updateStatusbar(unsigned int newState)
{
    // Only do this if the flags actually changed
    if (newState == _activeModifierState)
    {
        return;
    }

    _activeModifierState = newState;

    std::string statusText("");

    if (_activeModifierState != 0)
    {
        wxutil::MouseButton::ForeachButton([&](unsigned int button)
        {
            unsigned int testFlags = _activeModifierState | button;

            std::set<std::string> toolNames;

            GlobalMouseToolManager().foreachGroup([&](IMouseToolGroup& group)
            {
                MouseToolStack tools = group.getMappedTools(testFlags);

                for (auto i : tools)
                {
                    toolNames.insert(i->getDisplayName());
                }
            });

            if (!toolNames.empty())
            {
                statusText += wxutil::Modifier::GetModifierString(_activeModifierState) + "-";
                statusText += wxutil::MouseButton::GetButtonString(testFlags) + ": ";
                statusText += string::join(toolNames, ", ");
                statusText += " ";
            }
        });
    }

    if (statusText.empty())
    {
        _hintCloseTimer.Stop();

        if (_hintPopup)
        {
            _hintPopup->Close();
            _hintPopup.reset();
        }

        return;
    }

    _hintCloseTimer.StartOnce(1000);

    if (!_hintPopup)
    {
        _hintPopup.reset(new ModifierHintPopup(GlobalMainFrame().getWxTopLevelWindow()));
        _hintPopup->Show();
    }

    _hintPopup->SetText(statusText);

    // Pass the call
    //GlobalStatusBarManager().setText(STATUS_BAR_ELEMENT, statusText);
}

void MouseToolManager::onCloseTimerIntervalReached(wxTimerEvent& ev)
{
    // If no modifiers are held anymore, close the popup
    bool modifierHeld = wxGetKeyState(WXK_SHIFT) || wxGetKeyState(WXK_CONTROL) || wxGetKeyState(WXK_ALT);

    if (modifierHeld)
    {
        ev.GetTimer().StartOnce(1000);
        return;
    }

    if (_hintPopup)
    {
        _hintPopup->Close();
        _hintPopup.reset();
        return;
    }
}

module::StaticModule<MouseToolManager> mouseToolManagerModule;

} // namespace
