
const char* Help = "composes .smpl files and .skeleton to a soundfont file.\n\
usage: sfcompose <pathToSkeleton> <pathToSmplFolder> <outfile> [{bankNumber} {presetNumber} ...]";

#if WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "dat/dat.h"
#include "sf3/mydef.h"
#include "sf3/sfont.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include "sf3/mysysinfo.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <list>
#include <set>
#include <unordered_map>
#include <fstream>
#include <filesystem>

struct SfDb {
	std::string sampleFolder;
	std::unordered_map<dat::Id, SfTools::Preset*> presets;
	std::unordered_map<dat::Id, SfTools::Instrument*> instruments;
	std::unordered_map<dat::Id, size_t> instrumentIndices;
	std::unordered_map<dat::Id, SfTools::Sample*> samples;
	std::unordered_map<dat::Id, size_t> sampleIndices;
	std::unordered_map<dat::Id, SfTools::Zone*> zones;
	std::unordered_map<SfTools::Sample*, const dat::SampleHeader*> sampleHeaders;
};

void read(const std::string& skeletonPath, dat::Skeleton& skeleton);
void writeHeader(const dat::Skeleton& skeleton, SfTools::SoundFont* sf);
void writePresets(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void writeInstruments(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void writeSamples(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void writeZones(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void linkInstrumentsToPresets(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void linkSamplesToInstruments(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void readSample(SfTools::Sample* sample, const SfDb& db, short *outBff, int length);
struct Preset {
	int bank = 0;
	int preset = 0;
};

typedef std::vector<Preset> PresetFilter;

std::unique_ptr<SfTools::SoundFont> load(const std::string& sfPath)
{
	auto sf = std::make_unique<SfTools::SoundFont>(sfPath);
	sf->read();
	return sf;
}

void saveAs(SfTools::SoundFont* sf, const std::string& newPath)
{
	QFile file(newPath);
	file.open(QFile::WriteOnly);
	sf->file = &file;
	try {
		sf->write();
	}
	catch (...) {
		file.close();
		sf->file = nullptr;
		throw;
	}
	file.close();
	sf->file = nullptr;
}

void process(const std::string& skeletonPath, const std::string& sampleFolder)
{
	dat::Skeleton skeleton;
	SfDb db;
	db.sampleFolder = sampleFolder;
	read(skeletonPath, skeleton);
	SfTools::SoundFont sf;
	using namespace std::placeholders;
	sf.readSampleFunction = std::bind(&readSample, _1, std::ref(db), _2, _3);
	writeHeader(skeleton, &sf);
	writePresets(skeleton, &sf, db);
	writeInstruments(skeleton, &sf, db);
	writeSamples(skeleton, &sf, db);
	writeZones(skeleton, &sf, db);
	linkInstrumentsToPresets(skeleton, &sf, db);
	linkSamplesToInstruments(skeleton, &sf, db);
	saveAs(&sf, "copy.sf2");
}

void printHelp()
{
	std::cout << Help << std::endl;
}

int main(int argc, const char** argv)
{
#if WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	try {
		if (argc >= 2 && std::string(argv[1]) == "--help") {
			printHelp();
		}
		if (argc < 4) {
			printHelp();
			return 0;
		}
		process(argv[1], argv[2]);
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return -1;
	}
	return 0;
}

template<typename T>
void readContainer(dat::Container<T>& container, std::fstream& file)
{
	size_t byteSize = 0;
	file >> byteSize;
	if (byteSize == 0) {
		return;
	}
	container.resize(byteSize / sizeof(T));
	file.read((char*)container.data(), byteSize);
}

void read(const std::string& skeletonPath, dat::Skeleton &skeleton)
{
	std::fstream file(skeletonPath.c_str(), std::ios_base::in | std::ios_base::binary);
	file.read((char*)&skeleton.header, sizeof(dat::SoundFontHeader));
	readContainer(skeleton.generators, file);
	readContainer(skeleton.modulators, file);
	readContainer(skeleton.presets, file);
	readContainer(skeleton.instruments, file);
	readContainer(skeleton.instrument2Preset, file);
	readContainer(skeleton.samples, file);
	readContainer(skeleton.sample2Instruments, file);
}


void getString(char** dst, const dat::StringType& source) {
	if (strlen(source) == 0) {
		*dst = nullptr;
		return;
	}
	*dst = strdup(&source[0]);
}

void writeHeader(const dat::Skeleton& skeleton, SfTools::SoundFont* sf)
{
	const auto header = skeleton.header;
	sf->version = skeleton.header.version;
	sf->iver = header.iver;
	getString(&sf->engine, header.engine);
	getString(&sf->name, header.name);
	getString(&sf->date, header.date);
	getString(&sf->comment, header.comment);
	getString(&sf->tools, header.tools);
	getString(&sf->creator, header.creator);
	getString(&sf->product, header.product);
	getString(&sf->copyright, header.copyright);
	getString(&sf->irom, header.irom);
}

void writePresets(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db)
{
	for (const auto& preset : skeleton.presets)
	{
		auto sfpreset = new SfTools::Preset();
		sf->presets.push_back(sfpreset);
		getString(&sfpreset->name, preset.name);
		sfpreset->preset = preset.preset;
		sfpreset->bank = preset.bank;
		sfpreset->presetBagNdx = preset.presetBagNdx;
		sfpreset->library = preset.library;
		sfpreset->genre = preset.genre;
		sfpreset->morphology = preset.morphology;
		db.presets.insert(std::make_pair(preset.id, sfpreset));
	}
}

void writeInstruments(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db)
{
	for (const auto& instrument : skeleton.instruments)
	{
		auto sfInstrument = new SfTools::Instrument();
		getString(&sfInstrument->name, instrument.name);
		sfInstrument->index = instrument.index;
		sf->instruments.push_back(sfInstrument);
		db.instruments.insert(std::make_pair(instrument.id, sfInstrument));
		db.instrumentIndices.insert(std::make_pair(instrument.id, sf->instruments.size() - 1));
	}
}

void writeSamples(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db)
{
	for (const auto& sample : skeleton.samples) {
		auto sfSample = new SfTools::Sample();
		getString(&sfSample->name, sample.name);
		sfSample->start = sample.start;
		sfSample->end = sample.end;
		sfSample->loopstart = sample.loopstart;
		sfSample->loopend = sample.loopend;
		sfSample->samplerate = sample.samplerate;
		sfSample->origpitch = sample.origpitch;
		sfSample->pitchadj = sample.pitchadj;
		sfSample->sampleLink = sample.sampleLink;
		sfSample->sampletype = sample.sampletype;
		sf->samples.push_back(sfSample);
		db.samples.insert(std::make_pair(sample.id, sfSample));
		db.sampleIndices.insert(std::make_pair(sample.id, sf->samples.size() - 1));
		db.sampleHeaders.insert(std::make_pair(sfSample, &sample));
	}
}

SfTools::Zone* getPresetZone(dat::Id presetId, dat::Id zoneId, SfTools::SoundFont* sf, SfDb& db)
{
	auto it = db.zones.find(zoneId);
	if (it != db.zones.end()) {
		return it->second;
	}
	auto zone = new SfTools::Zone();
	db.zones.insert(std::make_pair(zoneId, zone));
	db.presets[presetId]->zones.push_back(zone);
	sf->pZones.push_back(zone);
	return zone;
}

SfTools::Zone* getInstrumentZone(dat::Id instrumentId, dat::Id zoneId, SfTools::SoundFont* sf, SfDb& db)
{
	auto it = db.zones.find(zoneId);
	if (it != db.zones.end()) {
		return it->second;
	}
	auto zone = new SfTools::Zone();
	db.zones.insert(std::make_pair(zoneId, zone));
	db.instruments[instrumentId]->zones.push_back(zone);
	sf->iZones.push_back(zone);
	return zone;
}


void writeZones(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db)
{
	for (const auto& generator : skeleton.generators) {
		auto zone = generator.for_ == dat::ForInstrument ? getInstrumentZone(generator.relatedTo, generator.zone, sf, db)
			: getPresetZone(generator.relatedTo, generator.zone, sf, db);
		auto sfGen = new SfTools::GeneratorList();
		sfGen->amount.uword = generator.amount.uword;
		sfGen->gen = generator.gen;
		zone->generators.push_back(sfGen);
	}

	for (const auto& modulator : skeleton.modulators) {
		auto zone = modulator.for_ == dat::ForInstrument ? getInstrumentZone(modulator.relatedTo, modulator.zone, sf, db)
			: getPresetZone(modulator.relatedTo, modulator.zone, sf, db);
		auto sfMod = new SfTools::ModulatorList();
		sfMod->amount = modulator.amount;
		sfMod->dst = modulator.dst;
		sfMod->transform = ::Linear;
		zone->modulators.push_back(sfMod);
	}
}

void linkInstrumentsToPresets(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db)
{
	for (const auto& rel : skeleton.instrument2Preset) {
		if (db.instrumentIndices.find(rel.instrument) == db.instrumentIndices.end()) {
			throw std::runtime_error("instrument " + std::to_string(rel.instrument) + " not found");
		}
		auto zone = getPresetZone(rel.preset, rel.zone, sf, db);
		auto instrumentIndex = db.instrumentIndices[rel.instrument];
		auto gen = new SfTools::GeneratorList();
		gen->gen = ::Gen_Instrument;
		gen->amount.uword = (unsigned short)instrumentIndex;
		zone->generators.push_back(gen);
	}
}

void linkSamplesToInstruments(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db)
{
	for (const auto& rel : skeleton.sample2Instruments) {
		if (db.sampleIndices.find(rel.sample) == db.sampleIndices.end()) {
			throw std::runtime_error("sample " + std::to_string(rel.sample) + " not found");
		}
		auto zone = getInstrumentZone(rel.instrument, rel.zone, sf, db);
		auto sampleIndex = db.sampleIndices[rel.sample];
		auto gen = new SfTools::GeneratorList();
		gen->gen = ::Gen_SampleId;
		gen->amount.uword = (unsigned short)sampleIndex;
		zone->generators.push_back(gen);
	}
}

void readSample(SfTools::Sample* sample, const SfDb& db, short* outBff, int length)
{
	auto headerIt = db.sampleHeaders.find(sample);
	if (headerIt == db.sampleHeaders.end()) {
		throw std::runtime_error("sample header not found");
	}
	auto header = headerIt->second;
	
}