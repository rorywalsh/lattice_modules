#pragma once
#include "LatticeProcessorModule.h"
#include "Osc.h"
#include <iterator>
#include <sstream>

class TestToneProcessor : public LatticeProcessorModule
{
public:
    TestToneProcessor();
    
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
    
    const char* getModuleName() override {    return "Test Tone";     }

    const std::string getSampleJson();
    std::stringstream ss;

private:
	Aurora::Osc<float, Aurora::lookupi<float>> osc;
	std::vector<float> wave;
    std::vector<float> ampVector;
    std::vector<float> freqVector;
    std::vector<float> samples;
	float amp = .5f;
    float freq = 100;
    int blockCnt = 0;
    int sampleIndex = 0;
};

