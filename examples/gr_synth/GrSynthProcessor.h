#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "grain.h"
#include "Env.h"
#include <cstdlib>
#include <functional>
#include <iostream>


class GrSynthProcessor : public LatticeProcessorModule
{
    
public:
    GrSynthProcessor();
    
    void createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs) override;
    
    /* This function is called by he host to populate the parameter vector */
    void createParameters(std::vector<ModuleParameter> &parameters) override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const std::string& parameterID, float newValue) override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /* Called by the host when a note is started */
    void startNote(int midiNoteNumber, float velocity) override;
    
    /* Called by the host when a note is stoped */
    void stopNote (float velocity) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void processSynthVoice(float** buffer, int numChannels, std::size_t blockSize) override;
    
    /* Is a synth */
    bool isSynth() override
    {
        return true;
    }

    std::string getModuleName() override
    {
        return "Grain Synth";
    }

private:
    std::vector<float> wave;
    float att, dec, sus, rel;
    Aurora::GrainGen<float> grain;
    Aurora::Env<float> env;
    float amp;
	float sr;
    bool isNoteOn = false;
};
