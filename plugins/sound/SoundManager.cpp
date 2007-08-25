#include "SoundManager.h"
#include "SoundFileLoader.h"

#include "ifilesystem.h"
#include "archivelib.h"
#include "generic/callback.h"
#include "parser/DefTokeniser.h"

#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <stdlib.h> // for atoi

namespace sound
{

// Constructor
SoundManager::SoundManager() :
	_emptyShader("")
{
	// Pass a SoundFileLoader to the filesystem
	SoundFileLoader loader(*this);
	GlobalFileSystem().forEachFile(
		SOUND_FOLDER,			// directory 
		"sndshd", 				// required extension
		makeCallback1(loader),	// loader callback
		99						// max depth
	);  
}

// Enumerate shaders
void SoundManager::forEachShader(SoundShaderVisitor visitor) const {
	for (ShaderMap::const_iterator i = _shaders.begin();
		 i != _shaders.end();
		 ++i)
	{
		visitor(*(i->second));	
	}
}

bool SoundManager::playSound(const std::string& fileName) {
	// Make a copy of the filename
	std::string name = fileName; 
	
	// Try to open the file as it is
	ArchiveFile* file = GlobalFileSystem().openFile(name.c_str());
	std::cout << "Trying: " << name << "\n";
	if (file != NULL) {
		// File found, play it
		std::cout << "Found file: " << name << "\n";
		_soundPlayer.play(*file);
		file->release();
		return true;
	}
	
	std::string root = name;
	// File not found, try to strip the extension
	if (name.rfind(".") != std::string::npos) {
		root = name.substr(0, name.rfind("."));
	}
	
	// Try to open the .ogg variant
	name = root + ".ogg";
	std::cout << "Trying: " << name << "\n";
	file = GlobalFileSystem().openFile(name.c_str());
	if (file != NULL) {
		std::cout << "Found file: " << name << "\n";
		_soundPlayer.play(*file);
		file->release();
		return true;
	}
	
	// Try to open the file with .wav extension
	name = root + ".wav";
	std::cout << "Trying: " << name << "\n";
	file = GlobalFileSystem().openFile(name.c_str());
	if (file != NULL) {
		std::cout << "Found file: " << name << "\n";
		_soundPlayer.play(*file);
		file->release();
		return true;
	}
	
	// File not found
	return false;
}

void SoundManager::stopSound() {
	_soundPlayer.stop();
}

// Accept a string of shaders to parse
void SoundManager::parseShadersFrom(const std::string& contents) {
	
	// Construct a DefTokeniser to tokenise the string into sound shader decls
	parser::BasicDefTokeniser<std::string> tok(contents);
	while (tok.hasMoreTokens())
		parseSoundShader(tok);
}

const ISoundShader& SoundManager::getSoundShader(const std::string& shaderName) {
	ShaderMap::const_iterator found = _shaders.find(shaderName);
	
	// If the name was found, return it, otherwise return an empty shader object
	return (found != _shaders.end()) ? *found->second : _emptyShader;    
}

// Parse a single sound shader from a token stream
void SoundManager::parseSoundShader(parser::DefTokeniser& tok) {
	
	// Get the shader name
	std::string name = tok.nextToken();
	
	// Create a new shader with this name
	_shaders[name] = ShaderPtr(new SoundShader(name));
	
	// A definition block must start here
	tok.assertNextToken("{");
	
	std::string nextToken = tok.nextToken();
	int min = 0;
	int max = 0;
	while (nextToken != "}") {
		// Watch out for sound file definitions and min/max radii
		if (boost::algorithm::starts_with(nextToken, "sound/")) {
			// Add this to the list
			_shaders[name]->addSoundFile(nextToken);
		}
		if (nextToken == "minDistance") {
			nextToken = tok.nextToken();

			min = atoi(nextToken.data());
		}
		if (nextToken == "maxDistance") {
			nextToken = tok.nextToken();
			max = atoi(nextToken.data());
		}
		nextToken = tok.nextToken();
	}
	SoundRadii soundRadii(min, max);
	_shaders[name]->setSoundRadii(soundRadii);
}
} // namespace sound
