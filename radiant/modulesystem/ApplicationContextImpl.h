#ifndef APPLICATIONCONTEXT_H_
#define APPLICATIONCONTEXT_H_

#include "imodule.h"

	namespace {
			const std::string RKEY_APP_PATH = "user/paths/appPath";
			const std::string RKEY_HOME_PATH = "user/paths/homePath";
			const std::string RKEY_SETTINGS_PATH = "user/paths/settingsPath";
			const std::string RKEY_BITMAPS_PATH = "user/paths/bitmapsPath";
			const std::string RKEY_ENGINE_PATH = "user/paths/enginePath";
			const std::string RKEY_MAP_PATH = "user/paths/mapPath";
			const std::string RKEY_PREFAB_PATH = "user/paths/prefabPath";
	}

namespace module {

class ApplicationContextImpl :
	public ApplicationContext
{
	// The app + home paths
	std::string _appPath;
	std::string _homePath;
	std::string _settingsPath;
	std::string _bitmapsPath;
	
	// The path where the .map files are stored
	std::string _mapsPath;
	
public:
	/**
	 * Initialises the context with the arguments given to main().
	 */
	void initialise(int argc, char* argv[]);
	
	/**
	 * Return the application path of the current Radiant instance.
	 */
	virtual const std::string& getApplicationPath() const;
	
	/**
	 * Return the settings path of the current Radiant instance.
	 */
	virtual const std::string& getSettingsPath() const;
	
	/** 
	 * Return the path where the application's bitmaps are stored.
	 */
	virtual const std::string& getBitmapsPath() const;
	
	// Return the global stream references
	virtual TextOutputStream& getOutputStream() const;
	virtual TextOutputStream& getErrorStream() const;

	// Exports/deletes the paths to/from the registry
	void savePathsToRegistry();
	void deletePathsFromRegistry();
	
private:
	// Sets up the bitmap path and settings path
	void initPaths();

	// Initialises the arguments
	void initArgs(int argc, char* argv[]);
};

} // namespace module

#endif /*APPLICATIONCONTEXT_H_*/
