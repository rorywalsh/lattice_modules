
#include "OnePoleProcessor.h"
#include <iterator>
#include <sstream>


OnePoleProcessor::OnePoleProcessor():lpL(44100), lpR(44100)
{

}

void OnePoleProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");  
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}


void OnePoleProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({ "Frequency", {1, 22050, 100, 1, .5f}});
	//parameters.push_back({ "Bandwidth", {1, 22050, 100, 1, .5f}});
	//parameters.push_back({ "Output Gain", {0, 2.f, 0.5f, 0.01f, 1}});
}

void OnePoleProcessor::hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
{
//    ignoreParameters(parameterID, newValue);
}

void OnePoleProcessor::prepareProcessor(int sr, std::size_t)
{
	lpL.reset(sr);
	lpR.reset(sr);
}


void OnePoleProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}


void OnePoleProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData /*hostInfo*/)
{
    inL.resize(blockSize);
    inR.resize(blockSize);

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());

	const std::vector<float>& outL = lpL(inL, getParameter("Frequency"));
	const std::vector<float>& outR = lpR(inR, getParameter("Frequency"));

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
	__declspec(dllexport) LatticeProcessorModule* create() { return new OnePoleProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new OnePoleProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif