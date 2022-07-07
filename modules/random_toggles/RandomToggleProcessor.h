#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>

class RandomToggleProcessor : public LatticeProcessorModule
{
public:
    RandomToggleProcessor();
    
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
    
    const char* getModuleName() override {    return "Random Toggles";     }



private:
    float samplingRate = 44100;
	int returnValue1 = 0;
    int returnValue2 = 0;
    int returnValue3 = 0;
    int returnValue4 = 0;
    int sampleIndexTgl1 = 0;
    int sampleIndexTgl2 = 0;
    int sampleIndexTgl3 = 0;
    int sampleIndexTgl4 = 0;
    float randVal = 0;
};

