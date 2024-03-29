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
    void process(float** buffer, std::size_t blockSize) override;
    
    int getModuleType() override
    {
        return ModuleType::AudioProcessor::delay;
    }
    
    const char* getModuleName() override
    {
        return "Feedback Delay";
    }

    
private:
    Aurora::Del<float, Aurora::vdelayi> delayL;
    Aurora::Del<float, Aurora::vdelayi> delayR;
    std::vector<float> inL;
    std::vector<float> inR;
    ParamSmooth timeL, timeR;
    int samplingRate;
};
