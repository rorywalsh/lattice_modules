#pragma once
#include "LatticeProcessorModule.h"
#include "Del.h"
#include <iterator>

class DelayProcessor : public LatticeProcessorModule
{
public:
    DelayProcessor();
    
    LatticeProcessorModule::ChannelData  DelayProcessor::createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    LatticeProcessorModule::ParameterData createParameters() override;
    

    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    const char* getModuleName() override
    {
        return "Feedback Delay";
    }

    void createDescription(std::string& description)
    {
        description = "(with variable delay time and feedback)";
    }

private:
    Aurora::Del<float> delayL;
    Aurora::Del<float> delayR;
    std::vector<float> inL;
    std::vector<float> inR;
};

