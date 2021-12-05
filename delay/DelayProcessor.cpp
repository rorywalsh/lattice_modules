
#include "DelayProcessor.h"
#include <iterator>


DelayProcessor::DelayProcessor():
delayL(2, Aurora::fixed_delay<float>, 44100),
delayR(2, Aurora::fixed_delay<float>, 44100)
{
    
}

void DelayProcessor::createParameters(std::vector<ExternalParameter> &parameters)
{
    parameters.push_back({ "Delay Time", {0, 2, .1, .01, .5}});
    parameters.push_back({ "Feedback", {0, 1, .5, .01, .5}});
    parameters.push_back({ "Dry", {0, 1, .5, .01, .5}});
}

void DelayProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
    ignoreParameters(parameterID, newValue);
}

void DelayProcessor::prepareProcessor(int sr, int block)
{
    ignoreParameters(sr, block);
}


void DelayProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void DelayProcessor::process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues)
{
    inL.resize(blockSize);
    inR.resize(blockSize);

    memcpy(&inL[0], buffer[0], blockSize*sizeof(float));
    memcpy(&inR[0], buffer[1], blockSize*sizeof(float));
    
    const std::vector<float> &outL = delayL(inL, getParameter("Delay Time"), getParameter("Feedback"));
    const std::vector<float> &outR = delayR(inR, getParameter("Delay Time"), getParameter("Feedback"));

    for(int i = 0; i < blockSize ; i++)
    {
        buffer[0][i] = outL[i] + inL[i] * getParameter("Dry");
        buffer[1][i] = outR[i] + inR[i] * getParameter("Dry");
    }
}

// the class factories
extern "C" ExternalProcessor* create(){             return new DelayProcessor;         }
extern "C" void destroy(ExternalProcessor* p){      delete p;                     }

