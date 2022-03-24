
#include "EqProcessor.h"
#include <iterator>
#include <sstream>


EqProcessor::EqProcessor():eqL(44100), eqR(44100)
{

}

LatticeProcessorModule::ChannelData EqProcessor::createChannels()
{
    addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
    addChannel({"Input 2", LatticeProcessorModule::ChannelType::input});
    addChannel({"Output 1" , LatticeProcessorModule::ChannelType::output });
    addChannel({"Output 2", LatticeProcessorModule::ChannelType::input });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData EqProcessor::createParameters()
{
    addParameter({ "Frequency", {1, 22050, 100, 1, .5f}});
	addParameter({ "Bandwidth", {1, 22050, 100, 1, .5f}});
	addParameter({ "Output Gain", {0, 2.f, 0.5f, 0.01f, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void EqProcessor::prepareProcessor(int sr, std::size_t)
{
	eqL.reset(sr);
	eqR.reset(sr);
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