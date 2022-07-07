#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>

class RandomProcessor : public LatticeProcessorModule
{
public:
    RandomProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;

    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
	void process(float** buffer, std::size_t blockSize) override;
    
    void createDescription(std::string& description)
    {
        description = "";
    }

    
    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    int getModuleType() override
    {
        return ModuleType::Automator::uncategorised;
    }
    
    const char* getModuleName() override {    return "Random Generator (!)";     }



private:
    float samplingRate = 44100;
	float returnValue = 0;
    int sampleIndex = 0;
};

