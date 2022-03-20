
#include "TwoPoleProcessor.h"
#include <iterator>
#include <sstream>


TwoPoleProcessor::TwoPoleProcessor():lpL(44100), lpR(44100)
{

}

void TwoPoleProcessor::createChannelLayout(DynamicArray<const char*> &inputs, DynamicArray<const char*> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");    
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}


void TwoPoleProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({ "Filter Type", {0, 3.f, 0.f, 1.f, 1.f}});
    parameters.push_back({ "Frequency", {1, 22050, 100, 1, .5f}});
	parameters.push_back({ "Damping", {0, 2, 1, 0.001f, 1.f}});
    parameters.push_back({ "Overdrive", {0, 5, 0, 0.0001f, 1.f}});
	
}

void TwoPoleProcessor::hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
{
//    ignoreParameters(parameterID, newValue);
}

void TwoPoleProcessor::prepareProcessor(int sr, std::size_t block)
{
  	lpL.reset(sr);
	lpR.reset(sr);
}


void TwoPoleProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}


void TwoPoleProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData /*hostInfo*/)
{
    inL.resize(blockSize);
    inR.resize(blockSize);

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());

	const std::vector<float>& outL = lpL(inL, getParameter("Frequency"), getParameter("Damping"), getParameter("Overdrive"));
	const std::vector<float>& outR = lpR(inR, getParameter("Frequency"), getParameter("Damping"), getParameter("Overdrive"));

    for(int i = 0; i < blockSize ; i++)
    {
		buffer[0][i] = outL[i];
		buffer[1][i] = outR[i];
    }
}

// the class factories
#ifdef WIN32
    extern "C" 
    {
        __declspec(dllexport) LatticeProcessorModule* create() { return new TwoPoleProcessor; }
    };

    extern "C" 
    {
        __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
    };
#else
    extern "C" LatticeProcessorModule* create(){     return new TwoPoleProcessor;         }
    extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif