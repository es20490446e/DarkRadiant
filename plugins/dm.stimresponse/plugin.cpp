#include "iplugin.h"
#include "ieventmanager.h"
#include "ieclass.h"
#include "iscenegraph.h"
#include "iuimanager.h"
#include "iregistry.h"
#include "iselection.h"
#include "qerplugin.h"

#include "generic/callback.h"
#include "SREditor.h" 

/**
 * API module to register the menu commands for the ObjectivesEditor class.
 */
class StimResponseAPI
: public IPlugin
{
public:
	STRING_CONSTANT(Name, "StimResponse");
	typedef IPlugin Type;

	/**
	 * API constructor, registers the commands in the Event manager and UI
	 * manager.
	 */
	StimResponseAPI() {
		
		// Add the callback event
		GlobalEventManager().addCommand(
			"StimResponseEditor", 
			FreeCaller<ui::StimResponseEditor::toggle>()
		);
	
		// Add the menu item
		IMenuManager& mm = GlobalUIManager().getMenuManager();
		mm.add("main/entity", 	// menu location path
				"StimResponse", // name
				ui::menuItem,	// type
				"Stim/Response...",	// caption
				"stimresponse.png",	// icon
				"StimResponseEditor"); // event name
	}
	
	/**
	 * SingletonModule requires a getTable() method, although for plugins it is
	 * unused.
	 */
	IPlugin* getTable() {
		assert(false);
	}	
};

/**
 * Dependencies class.
 */
class StimResponseDependencies : 
	public GlobalRegistryModuleRef,
	public GlobalEventManagerModuleRef,
	public GlobalUIManagerModuleRef,
	public GlobalRadiantModuleRef,
	public GlobalSelectionModuleRef,
	public GlobalSceneGraphModuleRef,
	public TypeSystemRef,
	public GlobalEntityClassManagerModuleRef
{ 
public:
	StimResponseDependencies() :
		GlobalEntityClassManagerModuleRef("doom3")
	{}
};

/* Required code to register the module with the ModuleServer.
 */

#include "modulesystem/singletonmodule.h"

typedef SingletonModule<StimResponseAPI,
						StimResponseDependencies> StimResponseModule;

extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
	// Static module instance
	static StimResponseModule _instance;
	
	// Initialise and register the module	
	initialiseModule(server);
	_instance.selfRegister();
}
