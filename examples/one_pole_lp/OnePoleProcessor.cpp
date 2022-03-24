
#include "OnePoleProcessor.h"
#include <iterator>
#include <sstream>


OnePoleProcessor::OnePoleProcessor():lpL(44100), lpR(44100)
{

}

LatticeProcessorModule::ChannelData OnePoleProcessor::createChannels()
{
    addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
    addChannel({"Input 2", LatticeProcessorModule::ChannelType::input });
    addChannel({"Output 1", LatticeProcessorModule::ChannelType::output });
    addChannel({"Output 2", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData OnePoleProcessor::createParameters()
{
    addParameter({ "Frequency", {1, 22050, 100, 1, .5f}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void OnePoleProcessor::prepareProcessor(int sr, std::size_t)
{
	lpL.reset(sr);
	lpR.reset(sr);
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