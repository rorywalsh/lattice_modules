#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>

#define SIZE 8

class Mixer8 : public LatticeProcessorModule
{
public:
    Mixer8();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, std::size_t blockSize) override;
 
    const char* getModuleName() override {    return "Mixer 8";     }
    
    int getModuleType() override
    {
        return ModuleType::AudioProcessor::mixer;
    }
private:
	static int remap(float value, float rangeMin, float rangeMax, float newRangeMin, float newRangeMax)
	{
		return static_cast<int>(newRangeMin + (value - rangeMin) * (newRangeMax - newRangeMin) / (rangeMax - rangeMin));
	}

    ParamSmooth gainSmooth[SIZE];
    ParamSmooth panSmooth[SIZE];
    std::vector<std::string> gainParams;
    std::vector<std::string> panParams;
    std::vector<std::string> inputs;
    float fs = 44100;

};

