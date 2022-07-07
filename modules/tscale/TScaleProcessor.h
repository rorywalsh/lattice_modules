#pragma once
#include "LatticeProcessorModule.h"
#include "SpecStream.h"
#include <iterator>

class TScaleProcessor : public LatticeProcessorModule
{
public:
    TScaleProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    

    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, std::size_t blockSize) override;
    
    const char* getModuleName() override
    {
        return "Spectral Timescale";
    }

    int getModuleType() override
    {
        return ModuleType::AudioProcessor::spectral;
    }
    
private:
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    std::vector<float> in;
    float fs;
    std::vector<std::vector<Aurora::specdata<float>>> buf;
    std::vector<Aurora::specdata<float>> out;
    float scrub;
    float rp;
    int wp;
    int framecount = 0;
};
