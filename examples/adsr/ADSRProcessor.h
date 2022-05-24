#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"
#include "Env.h"

class ADSRProcessor : public LatticeProcessorModule
{
public:
    ADSRProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;

    void hostParameterChanged(const char* parameterID, float newValue) override;

    const char* getModuleName() override {    return "ADSR";     }   
    
    ModuleType getModuleType() override
    {
        return ModuleType::automator;
    }
    
private:
    float att, dec, sus;
    Aurora::Env<float> env;
    bool retrig;
    ParamSmooth smooth;
};

