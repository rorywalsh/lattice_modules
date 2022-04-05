#pragma once
#include "LatticeProcessorModule.h"
#include "SpecStream.h"
#include <iterator>

class FreezeProcessor : public LatticeProcessorModule
{
public:
    FreezeProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    

    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    const char* getModuleName() override
    {
        return "Spectral Freeze";
    }

private:
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    std::vector<float> in;
    std::vector<Aurora::specdata<float>> buf;
    int framecount = 0;
};
