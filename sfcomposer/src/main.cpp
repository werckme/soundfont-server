#if WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif


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


void saveAs(SfTools::SoundFont *sf, const std::string& newPath)
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

bool contains(const SfTools::Preset* what, const PresetFilter &where) {
	return std::find_if(where.begin(), where.end(), [what](const auto& x) {
		return x.preset == what->preset && x.bank == what->bank;
	}) != where.end();
}

std::list<const SfTools::Instrument*> getRelatedInstruments(const SfTools::SoundFont* source, const std::list<const SfTools::Preset*> &presets)
{
	std::set<ushort> instrumentsIds;
	for (const auto* preset : presets) {
		for (const auto* zone : preset->zones) {
			for (const auto* gen : zone->generators) {
				if (gen->gen == SfTools::Gen_Instrument) {
					instrumentsIds.insert(gen->amount.uword);
				}
			}
		}
	}
	std::list<const SfTools::Instrument*> result;
	for (auto instrumentId : instrumentsIds) {
		result.push_back(source->instruments.at(instrumentId));
	}
	return result;
}

std::list<const SfTools::Sample*> getRelatedSamples(const SfTools::SoundFont* source, const std::list<const SfTools::Instrument*> &instruments)
{
	std::set<ushort> sampleIds;
	for (const auto* instrument : instruments) {
		for (const auto* zone : instrument->zones) {
			for (const auto* gen : zone->generators) {
				if (gen->gen == SfTools::Gen_SampleId) {
					sampleIds.insert(gen->amount.uword);
				}
			}
		}
	}
	std::list<const SfTools::Sample*> result;
	for (auto sampleId : sampleIds) {
		result.push_back(source->samples.at(sampleId));
	}
	return result;
}

std::list<const SfTools::Preset*> filterPresets(const SfTools::SoundFont* source, const PresetFilter &filter)
{
	std::list<const SfTools::Preset*> keptPresets;
	for (const auto* preset : source->presets) {
		bool match = contains(preset, filter);
		if (!match) {
			continue;
		}
		keptPresets.push_back(preset);
	}
	return keptPresets;
}

void process(const std::string& sfPath, const std::string& outPath, const PresetFilter &keep)
{
	auto sf = load(sfPath);
	//auto keptPresets = filterPresets(sf.get(), keep);
	//auto keptInstruments = getRelatedInstruments(sf.get(), keptPresets);
	//auto keptSamples = getRelatedSamples(sf.get(), keptInstruments);
	//SfTools::SoundFont copy(outPath.c_str());
	//copy.samples = QList<SfTools::Sample*>(keptSamples.begin(), keptSamples.end());

	//saveAs(sf.get(), "copy.sf2");
}


int main(int argc, const char** argv)
{
#if WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	try {
		if (argc < 2) {
			throw std::runtime_error(
				"usage: sfcomposer <path to sf>"
			);
		}
		process(argv[1], "copy.sf2", { {0, 16} });
	} catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return -1;
	}
	return 0;
}