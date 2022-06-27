#pragma once
#include "LatticeProcessorModule.h"
#include "Del.h"
#include <iterator>

class DelayProcessor : public LatticeProcessorModule
{
public:
    DelayProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    

    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    int getModuleType() override
    {
        return ModuleType::AudioProcessor::delay;
    }
    
    const char* getModuleName() override
    {
        return "Four Tap Delay";
    }

    
private:
    Aurora::Del<float> delay;
    Aurora::Tap<float> tap;
    std::vector<float> in;
    std::vector<ParamSmooth> dt;
    int samplingRate;
};
