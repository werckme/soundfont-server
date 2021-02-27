#ifndef DAT_H
#define DAT_h

#include <com.h>
#include <vector>
#include <string>

/*
	representation for soundfont splits:
	gm.sf.skeleton: the sf header without the sample data
	gm.sf.{sampleId}.smpl: the sample data
*/

namespace dat {
	template <typename T>
	using Container = std::vector<T>;
	typedef int Id;
	enum { Unknown = -1 };
    enum For { ForUndefined, ForPreset, ForInstrument };
	struct Modulator {
		Id relatedTo = Unknown;
		For for_ = ForUndefined;
		::Generator dst = Gen_StartAddrOfs;
		int amount = 0;
	};
	union GeneratorAmount {
		short sword;
		unsigned short uword;
		struct {
			unsigned char lo, hi;
		};
	};
	struct Generator {
		Id relatedTo = Unknown;
		For for_ = ForUndefined;
		::Generator gen = Gen_StartAddrOfs;
		GeneratorAmount amount = {0};
	};

	struct Preset {
		Id id = Unknown;
		std::string name;
		int preset = 0;
		int bank = 0;
		int presetBagNdx = 0;
		int library = 0;
		int genre = 0;
		int morphology = 0;
	};

	struct Instrument {
		Id id = Unknown;
		std::string name;
		int index = 0;
	};

	struct Instrument2Preset
	{
		Id instrument = Unknown;
		Id preset = Unknown;
	};

	struct SampleHeader {
		Id id = Unknown;
		std::string name;
		unsigned int start = 0;
		unsigned int end = 0;
		unsigned int loopstart =  0;
		unsigned int loopend = 0;
		unsigned int samplerate = 0;

		int origpitch = 0;
		int pitchadj = 0;
		int sampleLink = 0;
		int sampletype = 0;
	};

	struct Sample2Instrument
	{
		Id instrument = Unknown;
		Id sample = Unknown;
	};

	struct SoundFontHeader {
		sfVersionTag version = {0};
		std::string engine;
		std::string name;
		std::string date;
		std::string comment;
		std::string tools;
		std::string creator;
		std::string product;
		std::string copyright;
		std::string irom;
		sfVersionTag iver = {0};
	};

	struct Skeleton {
		SoundFontHeader header;
		Container<Generator> generators;
		Container<Modulator> modulators;
		Container<Preset> presets;
		Container<Instrument> instruments;
		Container<Instrument2Preset> instrument2Preset;
		Container<SampleHeader> samples;
		Container<Sample2Instrument> sample2Instruments;
	};
}

#endif