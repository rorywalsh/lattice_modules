#pragma once
#include "LatticeProcessorModule.h"
#include "SpecStream.h"
#include "SpecShift.h"
#include "SpecPitch.h"
#include "Del.h"
#include <iterator>

class ATuneProcessor : public LatticeProcessorModule
{
public:
    ATuneProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;

    void hostParameterChanged(const char* parameterID, float newValue) override;

    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    const char* getModuleName() override
    {
        return "Spectral Tuner";
    }

private:
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    Aurora::SpecShift<float> shift;
    Aurora::SpecPitch<float> ptrack;
    std::vector<float> in;
    std::vector<bool> on;
    std::size_t framecount = 0;
    float scl = 1.;
    float fs = Aurora::def_sr;
    std::vector<std::string> labels;
};
