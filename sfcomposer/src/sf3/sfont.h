//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef SFONT_H
#define SFONT_H

#include "mydef.h"
#include "myclasses.h"
#include <com.h>
namespace SfTools {

	//---------------------------------------------------------
	//   ModulatorList
	//---------------------------------------------------------

	struct ModulatorList {
		Modulator src;
		Generator dst;
		int amount;
		Modulator amtSrc;
		Transform transform;
	};

	//---------------------------------------------------------
	//   GeneratorList
	//---------------------------------------------------------

	union GeneratorAmount {
		short sword;
		ushort uword;
		struct {
			uchar lo, hi;
		};
	};

	struct GeneratorList {
		Generator gen;
		GeneratorAmount amount;
	};

	//---------------------------------------------------------
	//   Zone
	//---------------------------------------------------------

	struct Zone {
		QList<GeneratorList*> generators;
		QList<ModulatorList*> modulators;
		int instrumentIndex;
		~Zone() {
			for (auto x : this->generators) {
				delete x;
			}
			for (auto x : this->modulators) {
				delete x;
			}
		}
		Zone* clone() const {
			auto res = new Zone();
			*res = *this;
			res->generators = QList<GeneratorList*>();
			res->modulators = QList<ModulatorList*>();
			for (auto* gen : generators) {
				auto* copy = new GeneratorList();
				*copy = *gen;
				res->generators.push_back(copy);
			}
			for (auto* mod : modulators) {
				auto* copy = new ModulatorList();
				*copy = *mod;
				res->modulators.push_back(copy);
			}
			return res;
		}
	};

	//---------------------------------------------------------
	//   Preset
	//---------------------------------------------------------

	struct Preset {
		char* name;
		int preset;
		int bank;
		int presetBagNdx; // used only for read
		int library;
		int genre;
		int morphology;
		QList<Zone*> zones;

		Preset() :name(nullptr), preset(0), bank(0), presetBagNdx(0), library(0), genre(0), morphology(0) {}
		~Preset() {
			free(name);
			for (auto x : this->zones) {
				delete x;
			}
		}
		Preset* clone() const {
			auto* res = new Preset();
			*res = *this;
			res->zones = QList<Zone*>();
			if (name) {
				res->name = strdup(name);
			}
			for (auto* zone : zones) {
				auto* copy = zone->clone();
				res->zones.push_back(copy);
			}
			return res;
		}
	};

	//---------------------------------------------------------
	//   Instrument
	//---------------------------------------------------------

	struct Instrument {
		char* name;
		int index;        // used only for read
		QList<Zone*> zones;

		Instrument();
		~Instrument();

		Instrument* clone() const {
			auto* res = new Instrument();
			*res = *this;
			res->zones = QList<Zone*>();
			if (name) {
				res->name = strdup(name);
			}
			for (auto* zone : zones) {
				auto* copy = zone->clone();
				res->zones.push_back(copy);
			}
			return res;
		}
	};

	//---------------------------------------------------------
	//   Sample
	//---------------------------------------------------------

	struct Sample {
		char* name;
		uint start;
		uint end;
		uint loopstart;
		uint loopend;
		uint samplerate;

		int origpitch;
		int pitchadj;
		int sampleLink;
		int sampletype;

		Sample();
		~Sample();

		Sample* clone() const {
			auto* res = new Sample();
			*res = *this;
			if (name) {
				res->name = strdup(name);
			}
			return res;
		}

	};

	//---------------------------------------------------------
	//   SoundFont
	//---------------------------------------------------------

	struct SoundFont {
		QString path;
		sfVersionTag version;
		char* engine;
		char* name;
		char* date;
		char* comment;
		char* tools;
		char* creator;
		char* product;
		char* copyright;
		char* irom;
		sfVersionTag iver;

		int samplePos;
		int sampleLen;

		QList<Preset*> presets;
		QList<Instrument*> instruments;

		QList<Zone*> pZones;
		QList<Zone*> iZones;
		QList<Sample*> samples;

		QFile* file;

		// Extra option
		bool _smallSf;
		unsigned readDword();
		int readWord();
		int readShort();
		int readByte();
		int readChar();
		int readFourcc(const char*);
		int readFourcc(char*);
		void readSignature(const char* signature);
		void readSignature(char* signature);
		void skip(int);
		void readSection(const char* fourcc, int len);
		void readVersion(sfVersionTag* v);
		char* readString(int);
		void readPhdr(int);
		void readBag(int, QList<Zone*>*);
		void readMod(int, QList<Zone*>*);
		void readGen(int, QList<Zone*>*);
		void readInst(int);
		void readShdr(int);

		void writeDword(int);
		void writeWord(unsigned short int);
		void writeByte(unsigned char);
		void writeChar(char);
		void writeShort(short);
		void write(const char* p, int n);
		void writeSample(const Sample*);
		void writeStringSection(const char* fourcc, char* s);
		void writePreset(int zoneIdx, const Preset*);
		void writeModulator(const ModulatorList*);
		void writeGenerator(const GeneratorList*);
		void writeInstrument(int zoneIdx, const Instrument*);

		void writeIfil();
		void writeIver();
		void writeSmpl();
		void writePhdr();
		void writeBag(const char* fourcc, QList<Zone*>*);
		void writeMod(const char* fourcc, const QList<Zone*>*);
		void writeGen(const char* fourcc, QList<Zone*>*);
		void writeInst();
		void writeShdr();

		int copySample(Sample* s);
		bool write();

		SoundFont(const QString& = "");
		~SoundFont();
		bool read();
	};
}
#endif