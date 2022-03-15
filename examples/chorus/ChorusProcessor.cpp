
#include "ChorusProcessor.h"
#include <iterator>


ChorusProcessor::ChorusProcessor()
  : chorus(Aurora::def_sr), inL(Aurora::def_vsize), inR(Aurora::def_vsize)
{
    
}

void ChorusProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({ "Delay Time (L)", {0, 5, .017f, .001f, 1}});
    parameters.push_back({ "LFO Frequency (L)", {0, 10, .93, .001f, 1}});
    parameters.push_back({ "Delay Time (R)", {0, 5, .013f, .001f, 1}});
	parameters.push_back({ "LFO Frequency (R)", {0, 10, .083f, .001f, 1} });
}

void ChorusProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
   unused(parameterID, newValue);
}

void ChorusProcessor::prepareProcessor(int sr, std::size_t block)
{
	chorus.reset(sr);
	chorus.vsize(block);
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

void ChorusProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData)
{
   
    inL.resize(blockSize);
    inR.resize(blockSize);
    chorus.vsize(blockSize);
    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());
    

    auto &l = chorus(inL, getParameter("LFO Frequency (L)"), getParameter("Delay Time (L)"), 0);
    auto &r = chorus(inR, getParameter("LFO Frequency (R)"), getParameter("Delay Time (R)"), 1);

    for(int i = 0; i < blockSize ; i++)
    {
        buffer[0][i] = l[i] * 0.1 - r[i] * 0.13;
        buffer[1][i] = r[i] * 0.1 - l[i] * 0.13;
    }
    
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new ChorusProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new ChorusProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
