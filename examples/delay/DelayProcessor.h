#pragma once
#include "LatticeProcessorModule.h"
#include "Del.h"
#include <iterator>

class DelayProcessor : public LatticeProcessorModule
{
public:
    DelayProcessor();
    
    void createChannelLayout(DynamicArray<const char*> &inputs, DynamicArray<const char*> &outputs) override;
    
    /* This function is called by he host to populate the parameter vector */
    void createParameters(std::vector<ModuleParameter> &parameters) override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const std::string& parameterID, float newValue);
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    std::string getModuleName() override
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

