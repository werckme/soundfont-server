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
#include <map>

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

typedef size_t Index;

std::list<SfTools::Instrument*> getRelatedInstruments(SfTools::SoundFont* source, std::list<SfTools::Preset*> &presets)
{
	std::vector<ushort> instrumentsIds;
	for (auto* preset : presets) {
		for (auto* zone : preset->zones) {
			for (auto* gen : zone->generators) {
				if (gen->gen == SfTools::Gen_Instrument) {
					auto idx = gen->amount.uword;
					auto alreadyInserted = std::find(instrumentsIds.begin(), instrumentsIds.end(), idx);
					if (alreadyInserted != instrumentsIds.end()) {
						gen->amount.uword = alreadyInserted - instrumentsIds.begin();
					} else {
						instrumentsIds.push_back(idx);
						gen->amount.uword = instrumentsIds.size() - 1;
					}
				}
			}
		}
	}
	std::list<SfTools::Instrument*> result;
	for (auto instrumentId : instrumentsIds) {
		result.push_back(source->instruments.at(instrumentId));
	}
	return result;
}

/*
	finds the related samples and sets the new index to the instrument
*/
std::list<SfTools::Sample*> getRelatedSamples(SfTools::SoundFont* source, std::list<SfTools::Instrument*> &instruments)
{
	std::vector<ushort> sampleIds;
	for (auto* instrument : instruments) {
		for (auto* zone : instrument->zones) {
			for (auto* gen : zone->generators) {
				if (gen->gen == SfTools::Gen_SampleId) {
					auto idx = gen->amount.uword;
					auto alreadyInserted = std::find(sampleIds.begin(), sampleIds.end(), idx);
					if (alreadyInserted != sampleIds.end()) {
						gen->amount.uword = alreadyInserted - sampleIds.begin();
					}
					else {
						sampleIds.push_back(idx);
						gen->amount.uword = sampleIds.size() - 1;
					}
				}
			}
		}
	}
	std::list<SfTools::Sample*> result;
	for (auto sampleId : sampleIds) {
		result.push_back(source->samples.at(sampleId));
	}
	return result;
}

std::list<SfTools::Preset*> filterPresets(const SfTools::SoundFont* source, const PresetFilter &filter)
{
	std::list<SfTools::Preset*> keptPresets;
	for (auto* preset : source->presets) {
		bool match = filter.empty() ? true : contains(preset, filter);
		if (!match) {
			continue;
		}
		keptPresets.push_back(preset);
	}
	return keptPresets;
}

template<class T>
QList<T*> clone(std::list<T*> list)
{
	QList<T*> result;
	for (auto* x : list) {
		auto copy = x->clone();
		result.push_back(copy);
	}
	return result;
}

void fillEmptyFields(const SfTools::SoundFont *src, SfTools::SoundFont *dst) 
{
	dst->version = src->version;
	dst->engine = strdup(src->engine);
	dst->name = strdup(src->name);
	dst->date = strdup(src->date);
	dst->comment = strdup(src->comment);
	dst->tools = strdup(src->tools);
	dst->creator = strdup(src->creator);
	dst->product = strdup(src->product);
	dst->copyright = strdup(src->copyright);
	dst->irom = strdup(src->irom);
	dst->iver = src->iver;
	dst->samplePos = 0;
	dst->sampleLen = 0;
	dst->_smallSf = src->_smallSf;
	for (const auto* preset : dst->presets) {
		for (auto* zone : preset->zones) {
			dst->pZones.push_back(zone);
		}
	}
	for (const auto* instrument : dst->instruments) {
		for (auto* zone : instrument->zones) {
			dst->iZones.push_back(zone);
		}
	}
}

/*
	can only performed one time, after that the relation preset->instrument->sample is broken, since to keep things easy,
	the mapping preset->instrument, instrument->sample will be updated during getRelatedInstruments and getRelatedSamples
	and therefore the new indices will be written to the origin object
*/
void process(const std::string& sfPath, const std::string& outPath, const PresetFilter &keep)
{
	auto sf = load(sfPath);
	auto keptPresets = filterPresets(sf.get(), keep);
	auto keptInstruments = getRelatedInstruments(sf.get(), keptPresets);
	auto keptSamples = getRelatedSamples(sf.get(), keptInstruments);
	SfTools::SoundFont copy(sfPath.c_str());
	copy.samples = clone(keptSamples);
	copy.presets = clone(keptPresets);
	copy.instruments = clone(keptInstruments);
	fillEmptyFields(sf.get(), &copy);
	saveAs(&copy, "copy.sf2");
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
		process(argv[1], "copy.sf2", { {0, 16 } });
	} catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return -1;
	}
	return 0;
}