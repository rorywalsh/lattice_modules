#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include <set>

class MidiArpProcessor : public LatticeProcessorModule
{
public:
    MidiArpProcessor();
    
    ChannelData createChannels() override;
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
	void processMidi(float** buffer, int numChannels, std::size_t blockSize, const HostData data, std::vector<LatticeMidiMessage>& midiMessages) override;

    const char* getModuleName() override {    return "Midi Arpeggiator";     }
    
    int getRandomNote(int lowestNote, int range)
    {
        return rand() % range + lowestNote;
    }


    int getModuleType() override
    {
        return ModuleType::MidiProcessor::modifier;
    }

	static int remap(float value, float rangeMin, float rangeMax, float newRangeMin, float newRangeMax)
	{
		return static_cast<int>(newRangeMin + (value - rangeMin) * (newRangeMax - newRangeMin) / (rangeMax - rangeMin));
	}

private:
	int sampleIndex = 0;
    int samplingRate = 44100;
    int noteIndex = 0;
    int currentNote = 0;
    int lastNotePlayed = -1;
    int time;
    //std::vector<LatticeMidiMessage> notes;
    std::set<int> notes;
};

