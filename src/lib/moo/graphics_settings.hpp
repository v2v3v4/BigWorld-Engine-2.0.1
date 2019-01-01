/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GRAPHICS_SETTINGS_REGISTRY_HPP
#define GRAPHICS_SETTINGS_REGISTRY_HPP

#include "cstdmf/debug.hpp"
#include "cstdmf/guard.hpp"
#include <string>
#include <vector>
#include <map>

// Forward declarations
typedef SmartPointer<class DataSection> DataSectionPtr;

namespace Moo
{

/**
 *	Represents a set of graphics options in the game. Each GraphicsSetting 
 *	object offers a number of options. There is always one and only one 
 *	active option at any one time. This class is abstract and should be 
 *	derived by classes that want to expose some form of graphics settings 
 *	option. For examples on how to use this class, see moo/managed_effect.hpp 
 *	or the CallbackGraphicsSetting class.
 *
 *	An option can be selected via the selectOption method. Calling this 
 *	method triggers the onOptionSelected virtual function (unless the setting
 *	is flagged as delayed). Client systems must always override this function. 
 *	GraphicsSetting objects should be registered with GraphicsSettings::add().
 *	They can be listed with GraphicsSettings::settings().
 *
 *	Settings flagged with &lt;delayed> do not get their activeOption property and
 *	onOptionSelected virtual method triggered when their active option change. 
 *	Instead, the call is delayed until GraphicsSettings::commitPending() is 
 *	called. Settings should set this flag in two cases: (1) for closely related 
 *	settings that can gain from being processed as a batch (see 
 *	texture_manager.cpp for an example) and (2) for settings that may take a 
 *	while to process. This gives the interface programmer a chance to warn the 
 *	user that processing the new settings may take a while to complete before 
 *	the client application blocks for a few seconds (currently, there is no 
 *	support for displaying a progress bar while the settings are being 
 *	processed).
 *
 *	Settings flagged with &lt;needsRestart>, when their active option change,
 *	will set the global needsRestart flag in GraphicsSetting. Settings that
 *	require the client application to be restarted to come into effect should 
 *	set this flag to true (see managed_effect for an example). The global 
 *	needsRestart flag can be queried with GraphicsSetting::needsRestart().
 */
class GraphicsSetting : public ReferenceCount
{
public:
	typedef std::pair<std::string, std::pair<std::string, bool> > StringStringBool;
	typedef std::vector<StringStringBool> StringStringBoolVector;
	typedef SmartPointer<GraphicsSetting> GraphicsSettingPtr;
	typedef std::vector<GraphicsSettingPtr> GraphicsSettingVector;

	GraphicsSetting(
		const std::string & label, 
		const std::string & desc,
		int activeOption, 
		bool delayed,
		bool needsRestart);

	virtual ~GraphicsSetting() = 0;

	const std::string & label() const;
	const std::string & desc() const;

	int addOption(const std::string & label, const std::string & desc, bool isSupported);
	const StringStringBoolVector & options() const;

	bool selectOption(int optionIndex, bool force=false);
	virtual void onOptionSelected(int optionIndex) = 0;

	int activeOption() const;

	void needsRestart( bool a_needsRestart );

	void addSlave(GraphicsSettingPtr slave);

	static void init(DataSectionPtr section);
	static void write(DataSectionPtr section);

	static void add(GraphicsSettingPtr option);
	static const GraphicsSettingVector & settings();

	static bool hasPending();
	static bool isPending(GraphicsSettingPtr setting, int & o_pendingOption);
	static void commitPending();
	static void rollbackPending();

	static bool needsRestart();

	static GraphicsSettingPtr getFromLabel(
			const std::string & settingDesc);

private:
	bool selectOptionPriv(int optionIndex, bool force=false);

	std::string             label_;
	std::string				desc_;
	StringStringBoolVector  options_;
	int                     activeOption_;
	bool                    delayed_;
	bool                    needsRestart_;
	GraphicsSettingVector   slaves_;

	// static
	typedef std::pair<GraphicsSettingPtr, int> SettingIndexPair;
	typedef std::vector<SettingIndexPair> SettingIndexVector;
	typedef std::map<std::string, int> StringIntMap;

	static bool addPending(GraphicsSettingPtr setting, int optionIndex);

	static GraphicsSettingVector s_settings;
	static SettingIndexVector    s_pending;
	static StringIntMap          s_latentSettings;
	static bool                  s_needsRestart;
	static std::string           s_filename;
};

/**
 *	Implement the GraphicsSetting class, providing a member function 
 *	callback hook that is called whenever the active option changes. 
 *	This class provides the basic graphics settings functionality that 
 *	is needed by most client subsystems (see romp/flora.cpp for an 
 *	example).
 */
template<typename ClassT>
class CallbackGraphicsSetting : public GraphicsSetting
{
public:
	typedef void (ClassT::*Function)(int);

	CallbackGraphicsSetting(
			const std::string & label, 
			const std::string & desc, 
			ClassT            & instance, 
			Function            function,
			int                 activeOption,
			bool                delayed,
			bool                needsRestart) :
		GraphicsSetting(label, desc, activeOption, delayed, needsRestart),
		instance_(instance),
		function_(function)
	{}

	virtual void onOptionSelected(int optionIndex)
	{
		((&instance_)->*function_)(optionIndex);
	}

	ClassT & instance_;
	Function function_;
};

/**
 *	Helper template function used to create a CallbackGraphicsSetting object.
 *	Use this template function to avoid having to provide the type parameters
 *	required to instantiate a CallbackGraphicsSetting object.
 */
template<typename ClassT>
GraphicsSetting::GraphicsSettingPtr 
	makeCallbackGraphicsSetting(
		const std::string & label, 
		const std::string & desc, 
		ClassT & instance, 
		void(ClassT::*function)(int),
		int activeOption,
		bool delayed,
		bool needsRestart)
{
	BW_GUARD;
	return GraphicsSetting::GraphicsSettingPtr(
		new CallbackGraphicsSetting<ClassT>(
			label, desc, instance, function, 
			activeOption, delayed, needsRestart));
}

/**
 *	Implement the GraphicsSetting class, providing a static function 
 *	callback hook that is called whenever the active option changes. 
 *	This class provides the basic graphics settings functionality that 
 *	is needed by most clientsubsystems (see romp/foot_print_renderer.cpp 
 *	for an example).
 */
class StaticCallbackGraphicsSetting : public GraphicsSetting
{
public:
	typedef void (*Function)(int);

	StaticCallbackGraphicsSetting(
			const std::string & label, 
			const std::string & desc, 
			Function            function,
			int                 activeOption,
			bool                delayed,
			bool                needsRestart) :
		GraphicsSetting(label, desc, activeOption, delayed, needsRestart),
		function_(function)
	{}

	virtual void onOptionSelected(int optionIndex)
	{
		(*function_)(optionIndex);
	}

	Function function_;
};

}

#endif // GRAPHICS_SETTINGS_REGISTRY_HPP
