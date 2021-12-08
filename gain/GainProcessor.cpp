
#include "GainProcessor.h"
#include <iterator>


GainProcessor::GainProcessor()
{
    
}

void GainProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}

void GainProcessor::createParameters(std::vector<ExternalParameter> &parameters)
{
    ExternalParameter param{ "Gain", {0, 1, .1, 0.001, .5}}; // { min, max, increment, defaultValue, skew }
    parameters.push_back(param);
}

void GainProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
    //ignoreParameters(parameterID, newValue);
}

void GainProcessor::prepareProcessor(int sr, int block)
{
    //ignoreParameters(sr, block);
}

void GainProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void GainProcessor::process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues)
{
    for ( int i = 0 ; i < blockSize ; i++)
    {
        for ( int chan = 0 ; chan < numChannels; chan++)
        {
            buffer[chan][i] = buffer[chan][i] * getParameter("Gain");                
        }
    }
}

// the class factories
extern "C" LatticeProcessorModule* create(){             return new GainProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }

