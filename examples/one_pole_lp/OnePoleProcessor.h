#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "OnePole.h"

class OnePoleProcessor : public LatticeProcessorModule
{

public:
    OnePoleProcessor();
    

    LatticeProcessorModule::ChannelData OnePoleProcessor::createChannels() override;
    LatticeProcessorModule::ParameterData createParameters() override;
    

    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
	void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    void createDescription(std::string& description)
    {
        description = "";
    }


    const char* getModuleName() override {    return "One Pole Lowpass";     }
    



private:
	Aurora::OnePole<float> lpL, lpR;
    std::vector<float> inL;
    std::vector<float> inR;
};

