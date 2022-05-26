#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>

class BitCrunchProcessor : public LatticeProcessorModule
{
public:
    BitCrunchProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;

    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
	void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    void createDescription(std::string& description)
    {
        description = "";
    }

    int getModuleType() override
    {
        return ModuleType::AudioProcessor::nonlinear;
    }
    
    const char* getModuleName() override {    return "Bit Crunch";     }



private:
    
    float sampleRate = 44100;
};

