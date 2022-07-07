#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"
#include "Osc.h"

class LFOProcessor : public LatticeProcessorModule
{
public:
    LFOProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, std::size_t blockSize) override;

    const char* getModuleName() override {    return "LFO";     }   
    
    int getModuleType() override
    {
        return ModuleType::Automator::uncategorised;
    }
    
private:
    Aurora::Osc<float, Aurora::phase> lfo;
    std::vector<float> utri;
    std::vector<float> btri;
    std::vector<float> sine;
    std::vector<float> cosine;
    bool smp = false;
    float rnd  = 0;
};

