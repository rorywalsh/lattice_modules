
#include "GainProcessor.h"
#include <iterator>


GainProcessor::GainProcessor()
{
    
}

void GainProcessor::createParameters(std::vector<ExternalParameter> &parameters)
{
    ExternalParameter param{ "Gain", {0, 1, .1, 1, .5}}; // { min, max, increment, defaultValue, skew }
    parameters.push_back(param);
}

void GainProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
    ignoreParameters(parameterID, newValue);
}

void GainProcessor::prepareProcessor(int sr, int block)
{
    ignoreParameters(sr, block);
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
extern "C" ExternalProcessor* create(){             return new GainProcessor;         }
extern "C" void destroy(ExternalProcessor* p){      delete p;                     }

