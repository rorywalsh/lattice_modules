
#include "FlangerProcessor.h"
#include <iterator>


FlangerProcessor::FlangerProcessor()
: flanger(10, 44100)
{
    
}

LatticeProcessorModule::ChannelData FlangerProcessor::createChannelLayout()
{
    addChannel({"Input", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Output", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData FlangerProcessor::createParameters()
{
    addParameter({ "Max Delay", {0, 5, 2.5f, 0.1f, 1.f} });
    addParameter({ "LFO Frequency", {0, 20, .5f, .0001f, 1.f}});
    addParameter({ "Feedback", {0, 1, .7f, .0001f, 1.f}});
    addParameter({ "Gain", {0, 1, .5, .01f, 1.f}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void FlangerProcessor::prepareProcessor(int sr, std::size_t block)
{
	flanger.reset(sr);
}

void FlangerProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData)
{
    in.resize(blockSize);
    std::copy(buffer[0], buffer[0] + blockSize, in.begin());
    auto &out = flanger(in, getParameter("LFO Frequency"), getParameter("Feedback"), getParameter("Gain"), getParameter("Max Delay")/1000.f);
    std::copy(out.begin(),out.end(),buffer[0]);
    
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
