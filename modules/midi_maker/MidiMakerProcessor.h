#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>

class MidiMakerProcessor : public LatticeProcessorModule
{
public:
    MidiMakerProcessor();
    
    ChannelData createChannels() override;
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    void startNote(int noteNumber, float velocity) override;
    void stopNote(int noteNumber, float velocity) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
	void processMidi(std::size_t blockSize, std::vector<LatticeMidiMessage>& midiMessages) override;
    
	const char* getDescription() override
	{
		return "Midi Maker\n"
			"\n"
			"This node generates a simple Midi pattern. \n"
			"\n"
			"- Number of Notes in Loop : Number of notes in pattern (range 1 -32)\n"
			"- Tempo : Playback rate (range 0 - 20)\n"
			"- Lowest Note : Lowest possible Midi note (range 20 - 128)\n"
			"- Range in semitones : range of possible Midi notes (range 0 - 60)\n"
			"- Generate new pattern : will trigger a new pattern (trigger)\n"
			"- Permit silences : allows silences to appear in pattern (on/off)\n"
			"- Play Midi : cause the node to start outputting Midi note (on/off)\n"
			"\n"
			"Note that the pattern will only be updated once 'Generate new pattern' is triggered. ";
	}

    const char* getModuleName() override {    return "Midi Pattern Generator";     }
    
    int getRandomNote(int range)
    {
        return rand() % range;
    }

    int getRandomMajorNote(int range)
    {
        return major[rand() % range];
    }

    int getRandomMinorNote(int range)
    {
        return minor[rand() % range];
    }

	/* override this method if you want to draw to the Lattice generic editor viewport */
	const char* getSVGXml() override;

	/* override this method and return true if you wish to enable drawing on the generic editor viewport */
	bool canDraw() override { 
		auto draw = okToDraw;
		okToDraw = false;
		return draw;
	}

    int getModuleType() override
    {
        return ModuleType::MidiProcessor::generator;
    }

	static int remap(float value, float rangeMin, float rangeMax, float newRangeMin, float newRangeMax)
	{
		return static_cast<int>(newRangeMin + (value - rangeMin) * (newRangeMax - newRangeMin) / (rangeMax - rangeMin));
	}

private:
	int sampleIndex = 0;
    int lastNotePlayed = 0;
    int samplingRate = 44100;
    int noteIndex = 0;
    int majorIntervals[8] = { 2, 2, 1, 2, 2, 2, 1 };
    int minorIntervals[8] = { 2, 1, 2, 2, 1, 2, 2 };
    bool noteOn = false;
    
    std::vector<int> noteOns;
    std::vector<int> major;
    std::vector<int> minor;
    std::vector<int> chromaticNotes;
    std::vector<int> majorNotes;
    std::vector<int> minorNotes;
    std::vector<int> outgoingNotes;
    std::vector<int> amps;
    bool playNote = true;
	std::atomic<bool> canUpdate{ true };
	bool okToDraw = true;
    std::string svgText;
    std::vector<std::string> modes;
};

