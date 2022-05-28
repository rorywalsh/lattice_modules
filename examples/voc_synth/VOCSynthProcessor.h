#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "SpecStream.h"
#include "SpecPitch.h"
#include "SpecShift.h"
#include "Env.h"
#include <cstdlib>
#include <functional>
#include <iostream>


class VOCSynthProcessor : public LatticeProcessorModule
{
    
public:
    VOCSynthProcessor();
    
    ChannelData createChannels() override;

    ParameterData createParameters() override;
    
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
    
    int getNumberOfVoices() override
    {
        return 6;
    }

    float getTailOffTime() override
    {
        return getParameter("Release");
    }

    bool restrictBlockSize() override { return true; }
    
    
    int getModuleType() override
    {
        return ModuleType::SynthProcessor::spectral;
    }

    const char* getModuleName() override
    {
        return "SpecVoc Synth";
    }


private:
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    Aurora::SpecShift<float> shift;
    Aurora::SpecPitch<float> ptrack;
    std::vector<float> in;
    float att, dec, sus, rel;
    Aurora::Env<float> env;
    float fs = Aurora::def_sr;
    bool note_on = false;
};
