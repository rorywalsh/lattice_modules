
#include "TwoPoleProcessor.h"
#include <iterator>
#include <sstream>


TwoPoleProcessor::TwoPoleProcessor():lpL(44100), lpR(44100)
{

}

LatticeProcessorModule::ChannelData TwoPoleProcessor::createChannels()
{
   addChannel({"Input 1", LatticeProcessorModule::ChannelType::input });
   addChannel({"Input 2", LatticeProcessorModule::ChannelType::input });
   addChannel({"Output 1", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData TwoPoleProcessor::createParameters()
{
    addParameter({ "Filter Type", {0, 3.f, 0.f, 1.f, 1.f}});
    addParameter({ "Frequency", {1, 22050, 100, 1, .5f}});
	addParameter({ "Damping", {0, 2, 1, 0.001f, 1.f}});
    addParameter({ "Overdrive", {0, 5, 0, 0.0001f, 1.f}});
    return ParameterData(getParameters(), getNumberOfParameters());	
}


void TwoPoleProcessor::prepareProcessor(int sr, std::size_t block)
{
  	lpL.reset(sr);
	lpR.reset(sr);
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