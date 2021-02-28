
const char* Help = "composes .smpl files and .skeleton to a soundfont file.\n\
usage: sfcompose <pathToSkeleton> <pathToSmplFolder> <samplePathTemplate> <outfile> [{bankNumber} {presetNumber} ...]\n\
	   to get a list of all needed samples (ids): \n\
	   sfcompose --getsampleids [{bankNumber} {presetNumber} ...]\n\
";

#define EMPTY_FILTER_MEANS_ALL 0

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
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <sstream>

#ifdef WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

#ifdef __EMSCRIPTEN__
std::string tty;
#endif

namespace filter {
	struct Preset {
		int bank = 0;
		int preset = 0;
	};

	typedef std::vector<Preset> Presets;
	struct Filter {
		Presets keep;
		std::unordered_set<dat::Id> _presetsToKeep;
		std::unordered_set<dat::Id> _instrumentsToKeep;
		std::unordered_set<dat::Id> _samplesToKeep;
		bool _keep(dat::Id id, const std::unordered_set<dat::Id> &container) const
		{
#if EMPTY_FILTER_MEANS_ALL==1
			if (container.empty()) {
				return true;
			}
#endif
			return container.find(id) != container.end();
		}
		inline bool keepPreset(dat::Id id) const { return _keep(id, _presetsToKeep); }
		inline bool keepInstrument(dat::Id id) const { return _keep(id, _instrumentsToKeep); }
		inline bool keepSample(dat::Id id) const { return _keep(id, _samplesToKeep); }
	};
}

struct Options {
	std::string skeletonPath;
	std::string samplePathTemplate;
	std::string sampleFolder;
	std::string outfile;
	filter::Presets filter;
	bool printIds = false;
	bool valid = true;
};

struct SfDb {
	std::string sampleFolder;
	std::string samplePathTemplate;
	filter::Filter filter;
	std::unordered_map<dat::Id, SfTools::Preset*> presets;
	std::unordered_map<dat::Id, SfTools::Instrument*> instruments;
	std::unordered_map<dat::Id, size_t> instrumentIndices;
	std::unordered_map<dat::Id, SfTools::Sample*> samples;
	std::unordered_map<dat::Id, size_t> sampleIndices;
	std::unordered_map<dat::Id, SfTools::Zone*> zones;
	std::unordered_map<SfTools::Sample*, const dat::SampleHeader*> sampleHeaders;
};

void read(const std::string& skeletonPath, dat::Skeleton& skeleton);
filter::Filter createFilter(const filter::Presets& keep, const dat::Skeleton& skeleton);
void writeHeader(const dat::Skeleton& skeleton, SfTools::SoundFont* sf);
void writePresets(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void writeInstruments(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void writeSamples(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void writeZones(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void writeZonesSum(SfTools::SoundFont* sf);
void linkInstrumentsToPresets(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void linkSamplesToInstruments(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db);
void readSample(SfTools::Sample* sample, const SfDb& db, short *outBff, int length);
void printSampleIds(const filter::Filter& filter);
Options getOptions(int argc, const char** argv);

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

void process(const Options &options)
{
	dat::Skeleton skeleton;
	dat::Container<dat::Preset> tmp;
	SfDb db;
	read(options.skeletonPath, skeleton);
	db.filter = createFilter(options.filter, skeleton);
	if (options.printIds) {
		printSampleIds(db.filter);
		return;
	}
	db.sampleFolder = options.sampleFolder;
	db.samplePathTemplate = options.samplePathTemplate;
	if (db.sampleFolder.back() != PATH_SEP) {
		db.sampleFolder.push_back(PATH_SEP);
	}
	using namespace std::placeholders;
	SfTools::SoundFont sf;
	sf.readSampleFunction = std::bind(&readSample, _1, std::ref(db), _2, _3);
	writeHeader(skeleton, &sf);
	writePresets(skeleton, &sf, db);
	writeInstruments(skeleton, &sf, db);
	writeSamples(skeleton, &sf, db);
	writeZones(skeleton, &sf, db);
	linkInstrumentsToPresets(skeleton, &sf, db);
	linkSamplesToInstruments(skeleton, &sf, db);
	writeZonesSum(&sf);
	saveAs(&sf, options.outfile);
}

void printHelp()
{
	std::cout << Help << std::endl;
}

const char * create_c_str(const std::string &input) 
{
	auto bff = new char[input.length() + 1];
	strcpy(&bff[0], input.c_str());
	bff[input.length() - 1] = '\0';
	return &bff[0];
}


#ifdef __EMSCRIPTEN__
extern "C" const char * composejs(int argc, const char** argv)
{
	Options options = getOptions(argc, argv);
	if (!options.valid) {
		return create_c_str("{error: \"options invalid\"}");
	}
	process(options);
	return create_c_str(tty);

}
#endif

int main(int argc, const char** argv)
{
#ifdef __EMSCRIPTEN__
	return 0;
#endif
#if WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	try {
		Options options = getOptions(argc, argv);
		if (!options.valid) {
			printHelp();
			return -1;
		}
		process(options);
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
	file.read(reinterpret_cast<char*>(&byteSize), sizeof(size_t));
	if (byteSize == 0) {
		return;
	}
	auto size = byteSize / sizeof(T);
	container.resize(size);
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
		if (!db.filter.keepPreset(preset.id)) {
			continue;
		}
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
		if (!db.filter.keepInstrument(instrument.id)) {
			continue;
		}
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
		if (!db.filter.keepSample(sample.id)) {
			continue;
		}
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
	return zone;
}


void writeZones(const dat::Skeleton& skeleton, SfTools::SoundFont* sf, SfDb& db)
{
	for (const auto& generator : skeleton.generators) {
		bool keep = generator.for_ == dat::ForInstrument 
			? db.filter.keepInstrument(generator.relatedTo) : db.filter.keepPreset(generator.relatedTo);
		if (!keep) {
			continue;
		}
		auto zone = generator.for_ == dat::ForInstrument 
			? getInstrumentZone(generator.relatedTo, generator.zone, sf, db)
			: getPresetZone(generator.relatedTo, generator.zone, sf, db);
		auto sfGen = new SfTools::GeneratorList();
		sfGen->amount.uword = generator.amount.uword;
		sfGen->gen = generator.gen;
		zone->generators.push_back(sfGen);
	}

	for (const auto& modulator : skeleton.modulators) {
		bool keep = modulator.for_ == dat::ForInstrument 
			? db.filter.keepInstrument(modulator.relatedTo) : db.filter.keepPreset(modulator.relatedTo);
		if (!keep) {
			continue;
		}
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
		if (!db.filter.keepInstrument(rel.instrument) || !db.filter.keepPreset(rel.preset)) {
			continue;
		}
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
		if (!db.filter.keepSample(rel.sample) || !db.filter.keepInstrument(rel.instrument)) {
			continue;
		}
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
	auto byteSize = length * sizeof(short);
	if (headerIt == db.sampleHeaders.end()) {
		throw std::runtime_error("sample header not found");
	}
	auto header = headerIt->second;
	auto samplePath = db.sampleFolder + db.samplePathTemplate + std::to_string(header->id) + ".smpl";
	std::fstream file(samplePath.c_str(), std::ios_base::in | std::ios_base::binary);
	auto fsize = file.tellg();
	file.seekg(0, std::ios_base::end);
	fsize = file.tellg() - fsize;
	if (fsize != byteSize) {
		throw std::runtime_error(samplePath + " file size mismatch expected " + std::to_string(byteSize) + " but was " + std::to_string(fsize));
	}
	file.seekg(0, std::ios_base::beg);
	file.read((char*)outBff, byteSize);
}

filter::Filter createFilter(const filter::Presets& keep, const dat::Skeleton& skeleton)
{
	filter::Filter filter;
	filter.keep = keep;
	for (const auto& preset : skeleton.presets) {
		bool found = std::find_if(keep.begin(), keep.end(), [&preset](const auto& x) { 
			return x.bank == preset.bank && x.preset == preset.preset; 
		}) != keep.end();
		if (found) {
			filter._presetsToKeep.insert(preset.id);
		}
	}
	for (const auto& rel : skeleton.instrument2Preset) {
		bool found = filter.keepPreset(rel.preset);
		if (found) {
			filter._instrumentsToKeep.insert(rel.instrument);
		}
	}
	for (const auto& rel : skeleton.sample2Instruments) {
		bool found = filter.keepInstrument(rel.instrument);
		if (found) {
			filter._samplesToKeep.insert(rel.sample);
		}
	}
	return filter;
}

void writeZonesSum(SfTools::SoundFont* sf)
{
	for (auto* preset : sf->presets) {
		for (auto* zone : preset->zones) {
			sf->pZones.push_back(zone);
		}
	}

	for (auto* instrument : sf->instruments) {
		for (auto* zone : instrument->zones) {
			sf->iZones.push_back(zone);
		}
	}
}

void printSampleIds(const filter::Filter& filter)
{
#ifdef __EMSCRIPTEN__
	std::stringstream ss;
	auto &os = ss;
	os << "[";
#else
	auto &os = std::cout;
#endif
	bool first = true;
	for (auto sampleId : filter._samplesToKeep) {
		if (first) {
			os << sampleId;
			first = false;
			continue;
		}
		os << "," << sampleId;
	}
	os << std::endl;
#ifdef __EMSCRIPTEN__
	tty << "]";
	tty = os.str();
#endif
}

//const char* Help = "composes .smpl files and .skeleton to a soundfont file.\n\
//usage: sfcompose <pathToSkeleton> <pathToSmplFolder> <samplePathTemplate> <outfile> [{bankNumber} {presetNumber} ...]\n\
//	   to get a list of all needed samples (ids): \n\
//	   sfcompose <pathToSkeleton> --getsampleids  [{bankNumber} {presetNumber} ...]\n\
//";
Options getOptions(int argc, const char** argv)
{
	Options options;
	std::vector<dat::Id> ids;
	ids.reserve(argc);
	for (int i = 1; i < argc; ++i) {
		auto arg = std::string(argv[i]);
		if (arg == "--getsampleids") {
			options.printIds = true;
			continue;
		}
		if (i == 1 && !options.printIds) {
			options.skeletonPath = arg;
			continue;
		}
		if (i == 2 && !options.printIds) {
			options.sampleFolder = arg;
			continue;
		}
		if (i == 3 && !options.printIds) {
			options.samplePathTemplate = arg;
			continue;
		}
		if (i == 4 && !options.printIds) {
			options.outfile = arg;
			continue;
		}
		ids.push_back(atoi(arg.c_str()));
	}
	if (ids.empty() || ids.size() % 2 != 0) {
		options.valid = false;
	}
	if (options.skeletonPath.empty()) {
		options.valid = false;
	}
	if (!options.printIds && options.sampleFolder.empty()) {
		options.valid = false;
	}
	if (!options.printIds && options.samplePathTemplate.empty()) {
		options.valid = false;
	}
	if (!options.printIds && options.outfile.empty()) {
		options.valid = false;
	}
	if (!options.valid) {
		return options;
	}
	for (int i = 0; i < ids.size(); i += 2) {
		options.filter.push_back({ids.at(i), ids.at((int)(i+1))});
	}
	return options;
}