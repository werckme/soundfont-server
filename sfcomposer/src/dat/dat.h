#ifndef DAT_H
#define DAT_H

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
	enum { StringLength = 20 };
	typedef char StringType[StringLength];
	enum { Unknown = -1 };
    enum For { ForUndefined, ForPreset, ForInstrument };
	struct Modulator {
		Id relatedTo = Unknown;
		Id zone = Unknown;
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
		Id zone = Unknown;
		For for_ = ForUndefined;
		::Generator gen = Gen_StartAddrOfs;
		GeneratorAmount amount = {0};
	};

	struct Preset {
		Id id = Unknown;
		StringType name = { 0 };
		int preset = 0;
		int bank = 0;
		int presetBagNdx = 0;
		int library = 0;
		int genre = 0;
		int morphology = 0;
	};

	struct Instrument {
		Id id = Unknown;
		StringType name = { 0 };
		int index = 0;
	};

	struct Instrument2Preset
	{
		Id instrument = Unknown;
		Id preset = Unknown;
		Id zone = Unknown;
	};

	struct SampleHeader {
		Id id = Unknown;
		StringType name = { 0 };
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
		Id zone = Unknown;
	};

	struct SoundFontHeader {
		sfVersionTag version = { 0 };
		StringType engine = { 0 };
		StringType name = { 0 };
		StringType date = { 0 };
		StringType comment = { 0 };
		StringType tools = { 0 };
		StringType creator = { 0 };
		StringType product = { 0 };
		StringType copyright = { 0 };
		StringType irom = { 0 };
		sfVersionTag iver = { 0 };
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