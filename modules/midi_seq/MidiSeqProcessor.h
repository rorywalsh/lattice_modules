#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>

class MidiSeqProcessor : public LatticeProcessorModule
{
public:
    MidiSeqProcessor();
    
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
    
    const char* getModuleName() override {    return "Simple Midi Sequencer";     }
    
    int getModuleType() override
    {
        return ModuleType::MidiProcessor::generator;
    }

private:
	int sampleIndex = 0;
    int lastNotePlayed = 0;
    int samplingRate = 44100;
    int noteIndex = 0;
    int numSteps = 8;
    std::vector<std::string> noteParams;
    std::vector<std::string> velParams;
    
    bool playNote = true;
	std::atomic<bool> canUpdate{ true };
	bool okToDraw = true;
    std::string svgText;
    std::vector<std::string> chords;
};

