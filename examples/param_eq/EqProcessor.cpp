
#include "EqProcessor.h"
#include <iterator>
#include <sstream>


EqProcessor::EqProcessor():eqL(44100), eqR(44100)
{

}

void EqProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");    
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}


void EqProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({ "Frequency", {1, 22050, 100, 1, .5f}});
	parameters.push_back({ "Bandwidth", {1, 22050, 100, 1, .5f}});
	parameters.push_back({ "Output Gain", {0, 2.f, 0.5f, 0.01f, 1}});
}

void EqProcessor::hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
{
//    ignoreParameters(parameterID, newValue);
}

void EqProcessor::prepareProcessor(int sr, std::size_t)
{
	eqL.reset(sr);
	eqR.reset(sr);
}


void EqProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}


void EqProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    inL.resize(blockSize);
    inR.resize(blockSize);

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());

	auto& outL = eqL(inL, getParameter("Output Gain"), getParameter("Frequency"), getParameter("Bandwidth"));
    auto& outR = eqR(inR, getParameter("Output Gain"), getParameter("Frequency"), getParameter("Bandwidth"));

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
	__declspec(dllexport) LatticeProcessorModule* create() { return new EqProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new EqProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif