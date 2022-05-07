#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "SpecStream.h"
#include "SpecShift.h"
#include "Env.h"
#include <cstdlib>
#include <functional>
#include <iostream>


class SpecSampProcessor : public LatticeProcessorModule
{
    
public:
    SpecSampProcessor();
    
    ChannelData createChannels() override;

    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    void hostParameterChanged(const char* parameterID,
						 const char* newValue) override;
    
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
        return 1;
    }

        float getTailOffTime() override
    {
      return 0;
    }

    bool restrictBlockSize() override { return true; }
    
    
    ModuleType getModuleType() override
    {
        return ModuleType::synthProcessor;
    }

    const char* getModuleName() override
    {
        return "Spec Sampler";
    }

private:
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    Aurora::SpecShift<float> shift;
    std::vector<std::vector<Aurora::specdata<float>>> samp;
    float att, dec, sus, rel;
    Aurora::Env<float> env;
    std::size_t hcnt;
    float fs = Aurora::def_sr;
    std::size_t rp = 0;
    bool note_on = false;
};
