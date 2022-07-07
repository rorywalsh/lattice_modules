#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>

class MidiChordProcessor : public LatticeProcessorModule
{
public:
    MidiChordProcessor();
    
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
	void processMidi(std::size_t blockSize, std::vector<LatticeMidiMessage>& midiMessages) override;

    void addNotesToMidiBuffer(std::vector<LatticeMidiMessage>& midiMessages, int index);
    
    const char* getModuleName() override {    return "Midi Chord";     }
    
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
    std::vector<std::vector<int>> intervals;
	int sampleIndex = 0;
    int samplingRate = 44100;
    int time = 0;
    std::vector<int> notes;
    std::vector<LatticeMidiMessage> outgoingNotes;
    std::vector<std::string> chords;
    int chordIndex = 0;
};

