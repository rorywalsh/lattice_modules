#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"

class Mixer16 : public LatticeProcessorModule
{
public:
    Mixer16();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, std::size_t blockSize) override;
 
    const char* getModuleName() override {    return "Mixer16";     }
    
    int getModuleType() override
    {
        return ModuleType::AudioProcessor::mixer;
    }

private:
	static int remap(float value, float rangeMin, float rangeMax, float newRangeMin, float newRangeMax)
	{
		return static_cast<int>(newRangeMin + (value - rangeMin) * (newRangeMax - newRangeMin) / (rangeMax - rangeMin));
	}

    ParamSmooth gSmooth1, gSmooth2, gSmooth3, gSmooth4, pSmooth1, pSmooth2, pSmooth3, pSmooth4;
    float fs = 44100;

};

