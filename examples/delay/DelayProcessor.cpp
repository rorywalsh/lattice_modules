
#include "DelayProcessor.h"
#include <iterator>


DelayProcessor::DelayProcessor():
delayL(2, 44100),
delayR(2, 44100)
{
    
}

void DelayProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");    
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}


void DelayProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({ "Delay Time", {0, 2, .1f, .01f, 1}});
    parameters.push_back({ "Feedback", {0, 1, .5f, .01f, 1}});
    parameters.push_back({ "Dry", {0, 1, .5f, .01f, 1}});
    
    
}

void DelayProcessor::hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
{
//    ignoreParameters(parameterID, newValue);
}

void DelayProcessor::prepareProcessor(int /*sr*/, std::size_t/*block*/)
{
//    ignoreParameters(sr, block);
}


void DelayProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void DelayProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    inL.resize(blockSize);
    inR.resize(blockSize);

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());
                                
    const std::vector<float> &outL = delayL(inL, getParameter("Delay Time"), getParameter("Feedback"), getParameter("Dry"));
    const std::vector<float> &outR = delayR(inR, getParameter("Delay Time"), getParameter("Feedback"), getParameter("Dry"));

    for(std::size_t i = 0; i < blockSize ; i++)
    {
        buffer[0][i] = outL[i];
        buffer[1][i] = outR[i];
    }
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new DelayProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new DelayProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif