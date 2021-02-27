
const char * Help ="Extracts the sample data of a soundfont into several files.\n\
The header data will be saved as a skeleton file.\n\
usage: sfsplit <pathToSoundfont>";

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
#include <map>
#include <fstream>

void getHeader(const SfTools::SoundFont* sf, dat::Skeleton& out);
void getPresets(const SfTools::SoundFont* sf, dat::Skeleton& out);
void getInstruments(const SfTools::SoundFont* sf, dat::Skeleton& out);
void getSamples(const SfTools::SoundFont* sf, dat::Skeleton& out);
void getZones(const QList<SfTools::Zone*> zones, dat::Skeleton& out, dat::For for_, dat::Id id);
void writeSkeleton(const dat::Skeleton& skeleton, const std::string& path);
void writeSamples(const dat::Skeleton& skeleton, const SfTools::SoundFont* sf, const std::string& basePath);

int zoneIdCounter = -1;

std::unique_ptr<SfTools::SoundFont> load(const std::string& sfPath)
{
	auto sf = std::make_unique<SfTools::SoundFont>(sfPath);
	sf->read();
	return sf;
}

void getString(char* str, dat::StringType &dst) {
	if (str == nullptr) {
		return;
	}
	memcpy(&dst[0], str, dat::StringLength - 1);
	dst[dat::StringLength - 1] = 0;
}

void process(const std::string& sfPath)
{
	auto sf = load(sfPath);
	dat::Skeleton skeleton;
	getHeader(sf.get(), skeleton);
	getPresets(sf.get(), skeleton);
	getInstruments(sf.get(), skeleton);
	getSamples(sf.get(), skeleton);
	writeSkeleton(skeleton, sfPath + ".skeleton");
	writeSamples(skeleton, sf.get(), sfPath);
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
		if (argc < 2) {
			printHelp();
			return 0;
		}
		process(argv[1]);
	} catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return -1;
	}
	return 0;
}

void getHeader(const SfTools::SoundFont* sf, dat::Skeleton& out)
{
	dat::SoundFontHeader& header = out.header;
	getString(sf->engine, header.engine);
	getString(sf->name, header.name);
	getString(sf->date, header.date);
	getString(sf->comment, header.comment);
	getString(sf->tools, header.tools);
	getString(sf->creator, header.creator);
	getString(sf->product, header.product);
	getString(sf->copyright, header.copyright);
	getString(sf->irom, header.irom);
	header.version = sf->version;
	header.iver = sf->iver;
}

void getPresets(const SfTools::SoundFont* sf, dat::Skeleton& out)
{
	int idCounter = 0;
	for (const auto* preset : sf->presets) {
		dat::Preset outPreset;
		outPreset.id = idCounter++;
		getString(preset->name, outPreset.name);
		outPreset.preset = preset->preset;
		outPreset.bank = preset->bank;
		outPreset.presetBagNdx = preset->presetBagNdx;
		outPreset.library = preset->library;
		outPreset.genre = preset->genre;
		outPreset.morphology = preset->morphology;
		getZones(preset->zones, out, dat::ForPreset, outPreset.id);
		out.presets.push_back(outPreset);
	}
}

void getInstruments(const SfTools::SoundFont* sf, dat::Skeleton& out)
{
	int idCounter = 0;
	for (const auto* instrument : sf->instruments) {
		dat::Instrument outInstrument;
		outInstrument.id = idCounter++;
		getString(instrument->name, outInstrument.name);
		outInstrument.index = instrument->index;
		getZones(instrument->zones, out, dat::ForInstrument, outInstrument.id);
		out.instruments.push_back(outInstrument);
	}
}

void getSamples(const SfTools::SoundFont* sf, dat::Skeleton& out)
{
	int idCounter = 0;
	for (const auto* sample : sf->samples) {
		dat::SampleHeader outSample;
		outSample.id = idCounter++;
		getString(sample->name, outSample.name);
		outSample.start = sample->start;
		outSample.end = sample->end;
		outSample.loopstart = sample->loopstart;
		outSample.loopend = sample->loopend;
		outSample.samplerate = sample->samplerate;
		outSample.origpitch = sample->origpitch;
		outSample.pitchadj = sample->pitchadj;
		outSample.sampleLink = sample->sampleLink;
		outSample.sampletype = sample->sampletype;
		out.samples.push_back(outSample);
	}
}

void getZones(const QList<SfTools::Zone*> zones, dat::Skeleton& out, dat::For for_, dat::Id id)
{
	for (const auto* zone : zones) {
		++zoneIdCounter;
		for (const auto* generator : zone->generators) {
			dat::Generator outGenerator;
			outGenerator.relatedTo = id;
			outGenerator.for_ = for_;
			outGenerator.zone = zoneIdCounter;
			outGenerator.amount.uword = generator->amount.sword;
			outGenerator.gen = generator->gen;
			if (for_ == dat::ForPreset && outGenerator.gen == Gen_Instrument) {
				dat::Instrument2Preset i2p;
				i2p.preset = id;
				i2p.zone = zoneIdCounter;
				i2p.instrument = (dat::Id)outGenerator.amount.uword;
				out.instrument2Preset.push_back(i2p);
				continue;
			}
			if (for_ == dat::ForInstrument && outGenerator.gen == Gen_SampleId) {
				dat::Sample2Instrument s2i;
				s2i.instrument = id;
				s2i.zone = zoneIdCounter;
				s2i.sample = (dat::Id)outGenerator.amount.uword;
				out.sample2Instruments.push_back(s2i);
				continue;
			}
			out.generators.push_back(outGenerator);
		}
		for (const auto* modulator : zone->modulators) {
			dat::Modulator outModulator;
			outModulator.relatedTo = id;
			outModulator.for_ = for_;
			outModulator.amount = modulator->amount;
			outModulator.dst = modulator->dst;
			out.modulators.push_back(outModulator);
		}
	}
}

template<typename T>
void writeContainer(const dat::Container<T>& container, std::fstream &file)
{
	size_t byteSize = sizeof(T) * container.size();
	file << byteSize;
	file.write((const char*)container.data(), byteSize);
}

void writeSkeleton(const dat::Skeleton& skeleton, const std::string& path)
{
	std::fstream file(path.c_str(), std::ios_base::out | std::ios::binary);
	file.write((const char*)&skeleton.header, sizeof(dat::SoundFontHeader));
	writeContainer(skeleton.generators, file);
	writeContainer(skeleton.modulators, file);
	writeContainer(skeleton.presets, file);
	writeContainer(skeleton.instruments, file);
	writeContainer(skeleton.instrument2Preset, file);
	writeContainer(skeleton.samples, file);
	writeContainer(skeleton.sample2Instruments, file);
}

void writeSamples(const dat::Skeleton& skeleton, const SfTools::SoundFont* sf, const std::string& basePath)
{
	for (const auto& sampleHeader : skeleton.samples) {
		if (sampleHeader.start >= sampleHeader.end) {
			throw std::runtime_error("invalid sample length");
		}
		auto path = basePath + "." + std::to_string(sampleHeader.id) + ".smpl";
		size_t byteSize = sizeof(short) * (sampleHeader.end - sampleHeader.start);
		std::vector<char> bff(byteSize);
		std::fstream outfile(path.c_str(), std::ios_base::out | std::ios::binary);
		std::fstream infile(sf->path.c_str(), std::ios_base::in | std::ios::binary);
		infile.seekg(static_cast<size_t>(sf->samplePos) + sampleHeader.start);
		infile.read(bff.data(), byteSize);
		outfile.write(bff.data(), byteSize);
	}
}