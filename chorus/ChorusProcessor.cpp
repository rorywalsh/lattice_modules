
#include "ChorusProcessor.h"
#include <iterator>


ChorusProcessor::ChorusProcessor():chorus(44100)
{
    
}

void ChorusProcessor::createParameters(std::vector<ExternalParameter> &parameters)
{
    parameters.push_back({ "Delay Time", {0, 2, .1, .01, 1}});
    parameters.push_back({ "Feedback", {0, 1, .5, .01, 1}});
    parameters.push_back({ "Dry", {0, 1, .5, .01, 1}});
}

void ChorusProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
   // ignoreParameters(parameterID, newValue);
}

void ChorusProcessor::prepareProcessor(int sr, int block)
{
   // ignoreParameters(sr, block);
}


void ChorusProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void ChorusProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}

void ChorusProcessor::process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues)
{
   
    inL.resize(blockSize);
    inR.resize(blockSize);

    
    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());
    
    auto &l = chorus(inL, .93f, .017, 0);
    auto &r = chorus(inL, .87f, .013f, 1);

    for(int i = 0; i < blockSize ; i++)
    {
        buffer[0][i] = l[i] * 0.1 - r[i] * 0.13;
        buffer[1][i] = r[i] * 0.1 - l[i] * 0.13;;
    }
    
}

// the class factories
extern "C" LatticeProcessorModule* create(){             return new ChorusProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }

