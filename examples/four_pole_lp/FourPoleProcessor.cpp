
#include "FourPoleProcessor.h"
#include <iterator>
#include <sstream>


FourPoleProcessor::FourPoleProcessor():lpL(44100), lpR(44100)
{

}

void FourPoleProcessor::createChannelLayout(DynamicArray<const char*> &inputs, DynamicArray<const char*> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");    
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}


void FourPoleProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({ "Frequency", {1, 22050, 100, 1, .5f}});
	parameters.push_back({ "Resonance", {0, 1, 0, 0.001f, 1.f}});
}

void FourPoleProcessor::hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
{
//    ignoreParameters(parameterID, newValue);
}

void FourPoleProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
	lpL.reset(sr);
	lpR.reset(sr);
}


void FourPoleProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}


void FourPoleProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    inL.resize(static_cast<std::size_t>(blockSize));
    inR.resize(static_cast<std::size_t>(blockSize));

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());

	const std::vector<float>& outL = lpL(inL, getParameter("Frequency"), getParameter("Resonance"));
	const std::vector<float>& outR = lpR(inR, getParameter("Frequency"), getParameter("Resonance"));

    for(int i = 0; i < blockSize ; i++)
    {
		buffer[0][i] = outL[static_cast<std::size_t>(i)];
		buffer[1][i] = outR[static_cast<std::size_t>(i)];
    }
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new FourPoleProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(FourPoleProcessor* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new FourPoleProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif

