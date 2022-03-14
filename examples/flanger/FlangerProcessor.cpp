
#include "FlangerProcessor.h"
#include <iterator>


FlangerProcessor::FlangerProcessor()
: flangerL(10, 44100),
flangerR(10, 44100)
{
    
}

void FlangerProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({ "Max Delay", {0, 5, 2.5f, 0.1f, 1.f} });
    parameters.push_back({ "LFO Frequency", {0, 20, .5f, .0001f, 1.f}});
    parameters.push_back({ "Feedback", {0, 1, .7f, .0001f, 1.f}});
    parameters.push_back({ "Gain", {0, 1, .5, .01f, 1.f}});
}

void FlangerProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
   // ignoreParameters(parameterID, newValue);
}

void FlangerProcessor::prepareProcessor(int sr, std::size_t block)
{
   // ignoreParameters(sr, block);
}


void FlangerProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void FlangerProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}

void FlangerProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData)
{
   
    inL.resize(blockSize);
    inR.resize(blockSize);
    
    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());
    
    auto &l = flangerL(inL, getParameter("LFO Frequency"), getParameter("Feedback"), getParameter("Gain"), getParameter("Max Delay")/1000.f);
    auto &r = flangerR(inL, getParameter("LFO Frequency"), getParameter("Feedback"), getParameter("Gain"), getParameter("Max Delay")/1000.f);

    for(int i = 0; i < blockSize ; i++)
    {
        buffer[0][i] = l[i];
        buffer[1][i] = r[i];
    }
    
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new FlangerProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new FlangerProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
