#pragma once
#include "LatticeProcessorModule.h"
#include "SpecStream.h"
#include <iterator>

class FreezeProcessor : public LatticeProcessorModule
{
public:
    FreezeProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    void hostParameterChanged(const char* parameterID, float newValue) override;
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, std::size_t blockSize) override;
    
    const char* getModuleName() override
    {
        return "Spectral Freeze";
    }

    virtual int getModuleType() override
    {
        return ModuleType::AudioProcessor::spectral;
    }
    
private:
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    std::vector<float> in;
    std::vector<Aurora::specdata<float>> buf;
    std::vector<Aurora::specdata<float>> del;
    std::vector<Aurora::specdata<float>> out;
    float caf, cff;
    float ya, yf;
    float aa = 0 , af = 0;
    float smootha = 0.001, smoothf = 0.001;
    size_t timea = 0 , timef = 0, dtimea = 0 , dtimef = 0;
    bool freeza = 0 , freezf = 0;
    float ar;
    
    int framecount = 0;
};
