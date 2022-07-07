#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>

class MidiFilterProcessor : public LatticeProcessorModule
{
public:
    MidiFilterProcessor();
    
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
    
	const char* getDescription() override
	{
	  return "Midi Filter\n";
	}

    const char* getModuleName() override {    return "Midi Filter";     }
    
    int getRandomNote(int lowestNote, int range)
    {
        return rand() % range + lowestNote;
    }


    int getModuleType() override
    {
      return ModuleType::MidiProcessor::modifier;
    }

private:
    std::vector<LatticeMidiMessage> out;

};

