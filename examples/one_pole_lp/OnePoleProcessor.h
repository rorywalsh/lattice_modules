#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "OnePole.h"

class OnePoleProcessor : public LatticeProcessorModule
{

public:
    OnePoleProcessor();
    
    void createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs) override;
    
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
    
    void createDescription(std::string& description)
    {
        description = "";
    }


    std::string getModuleName() override {    return "One Pole Lowpass";     }
    



private:
	Aurora::OnePole<float> lpL, lpR;
    std::vector<float> inL;
    std::vector<float> inR;
};

