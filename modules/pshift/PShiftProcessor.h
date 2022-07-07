#pragma once
#include "LatticeProcessorModule.h"
#include "SpecStream.h"
#include "SpecShift.h"
#include "Del.h"
#include <iterator>

class PShiftProcessor : public LatticeProcessorModule
{
public:
    PShiftProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    

    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, std::size_t blockSize) override;
    
    int getModuleType() override
    {
        return ModuleType::AudioProcessor::spectral;
    }
    
    const char* getModuleName() override
    {
        return "Spectral Shifter";
    }

private:
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    Aurora::SpecShift<float> shift;
    Aurora::Del<float> delay;
    std::vector<float> in;
    float fs = Aurora::def_sr;
};
