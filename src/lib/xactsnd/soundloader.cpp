/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/watcher.hpp"
#include "resmgr/bwresource.hpp"

#include "soundmgr.hpp"
#include "soundloader.hpp"

using namespace std;

DECLARE_DEBUG_COMPONENT2( "Sound", -1 )	// debugLevel for this file




#if 0
static void loadFromXML_fx(DataSectionPtr dsp);
static void loadFromXML_simple(DataSectionPtr dsp);
static void loadFromXML_ambient(DataSectionPtr dsp);
static void loadFromXML_static3D(DataSectionPtr dsp);
#endif

void SoundLoader::loadFromXML(DataSectionPtr dataSection)
{
	TRACE_MSG("SoundLoader::loadFromXML:\n");
	MF_ASSERT(dataSection);
#if 0
	if (!dataSection)
		return;

	DataSection::iterator it;

	for (it=dataSection->begin();  it != dataSection->end();  it++)
	{
		DataSectionPtr dsp = *it;

		if		(dsp->sectionName() == "fx")		loadFromXML_fx(dsp);
		else if (dsp->sectionName() == "simple")	loadFromXML_simple(dsp);
		else if (dsp->sectionName() == "ambient")	loadFromXML_ambient(dsp);
		else if (dsp->sectionName() == "static3D")	loadFromXML_static3D(dsp);
		else {
			CRITICAL_MSG("Unknown sound type '%s'", dsp->sectionName().c_str());
		}
	}
#endif
}


#if 0
static void loadFromXML_fx(DataSectionPtr dsp)
{
	TRACE_MSG("::loadFromXML_fx:\n");

	string tag = dsp->readString("tag");
	if (tag.empty()) {
		CRITICAL_MSG("SoundFx is missing a tag, so can't be loaded");
		return;
	}

	string filename = dsp->readString("file");
	if (filename.empty()) {
		CRITICAL_MSG("SoundFx Tag '%s' is missing a filename, so can't be loaded", tag);
		return;
	}

	if (!::soundMgr().loadFx(
		filename.c_str(),
		tag.c_str(),
		dsp->readFloat("min", 5.0f),
		dsp->readFloat("max", 10.0f),
		dsp->readFloat("attenuation", 0.0f),
		dsp->readInt("maxInstances", 1),
		dsp->readBool("loop", false)
	)) {
		CRITICAL_MSG("SoundFx '%s - %s' failed to load",
				tag.c_str(), filename.c_str());
	}
}



static void loadFromXML_simple(DataSectionPtr dsp)
{
	TRACE_MSG("::loadFromXML_simple:\n");

	string filename = dsp->readString("file");
	if (filename.empty()) {
		CRITICAL_MSG("SimpleSound is missing a filename, so can't be loaded");
		return;
	}

	if (!::soundMgr().loadSimple(
		filename.c_str(),
		dsp->readFloat("attenuation", 0.0f),
		dsp->readInt("maxInstances", 1),
		dsp->readBool("loop", false)
	)) {
		CRITICAL_MSG("SimpleSound '%s' failed to load", filename.c_str());
	}
}



static void loadFromXML_ambient(DataSectionPtr dsp)
{
	TRACE_MSG("::loadFromXML_ambient:\n");
	string filename = dsp->readString("file");
	if (filename.empty()) {
		CRITICAL_MSG("AmbientSound is missing a filename, so can't be loaded");
		return;
	}

	vector<DataSectionPtr> emitSections;
	dsp->openSections("emitter", emitSections);
	vector<DataSectionPtr>::const_iterator emitIter;

	if (emitSections.size() == 0) {
		CRITICAL_MSG("AmbientSound: No emitters specified for sound %s", filename.c_str());
		return;
	}

	AmbientSound* snd = ::soundMgr().loadAmbient(
		filename.c_str(),
		emitSections[0]->readVector3("position"),
		emitSections[0]->readFloat(  "min", 5.0f),
		emitSections[0]->readFloat(  "max", 10.0f),
		dsp->readFloat(	"attenuation", 0.0f),
		dsp->readFloat( "outsideAtten", 0.0f),
		dsp->readFloat( "insideAtten", 0.0f),
		dsp->readFloat(	"centreHour", 0.0f),
		dsp->readFloat(	"minHour", 0.0f),
		dsp->readFloat(	"maxHour", 0.0f),
		dsp->readBool(  "loop", true),
		dsp->readFloat( "loopDelay", 0.0f),
		dsp->readFloat( "loopDelayVariance", 0.0f)
	);


	if (snd) {
		for (emitIter=emitSections.begin()+1;  emitIter != emitSections.end();  emitIter++) {
			DataSectionPtr edsp = *emitIter;
			snd->addPosition(	edsp->readVector3("position"),
								edsp->readFloat("min", 5.0f),
								edsp->readFloat("max", 10.0f));
		}
	} else {
		CRITICAL_MSG("AmbientSound '%s' failed to load", filename.c_str());
	}
}


static void loadFromXML_static3D(DataSectionPtr dsp)
{
	TRACE_MSG("::loadFromXML_static3D:\n");

	string filename = dsp->readString("file");
	if (filename.empty()) {
		CRITICAL_MSG("Static3DSound is missing a filename, so can't be loaded");
		return;
	}

	vector<DataSectionPtr> emitSections;
	dsp->openSections("emitter", emitSections);

	if (emitSections.size() == 0) {
		CRITICAL_MSG("Static3DSound: No emitters specified for sound %s", filename.c_str());
		return;
	}

	Static3DSound* snd = ::soundMgr().loadStatic3D(
		filename.c_str(),
		emitSections[0]->readVector3("position"),
		emitSections[0]->readFloat(	 "min", 5.0f),
		emitSections[0]->readFloat(	 "max", 10.0f),
		dsp->readFloat(	"attenuation", 0.0f),
		dsp->readFloat( "outsideAtten", 0.0f),
		dsp->readFloat( "insideAtten", 0.0f),
		dsp->readFloat(	"centreHour", 0.0f),
		dsp->readFloat(	"minHour", 0.0f),
		dsp->readFloat(	"maxHour", 0.0f),
		dsp->readBool(  "loop", true),
		dsp->readFloat( "loopDelay", 0.0f),
		dsp->readFloat( "loopDelayVariance", 0.0f)
	);

	if (!snd) {
		CRITICAL_MSG("Static3DSound '%s' failed to load", filename.c_str());
	}
}
#endif


/*soundloader.cpp*/
