#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "TwoPole.h"

inline float drive(float s, float drv) {
  return drv > 0.02 ? std::tanh(s*drv)/drv : s;
}

class TwoPoleProcessor : public LatticeProcessorModule
{

public:
    TwoPoleProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
	void process(float** buffer, std::size_t blockSize) override;
    
    virtual int getModuleType() override
    {
        return ModuleType::AudioProcessor::filters;
    }
    
    const char* getModuleName() override {    return "State Variable Filter";     }

private:
    Aurora::TwoPole<float,drive> lp;
    std::vector<float> in;
};

