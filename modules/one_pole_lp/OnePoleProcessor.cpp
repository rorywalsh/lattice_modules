
#include "OnePoleProcessor.h"
#include <iterator>
#include <sstream>


OnePoleProcessor::OnePoleProcessor():lp(44100)
{

}

LatticeProcessorModule::ChannelData OnePoleProcessor::createChannels()
{
    addChannel({ "Input", LatticeProcessorModule::ChannelType::input });
    addChannel({"Output", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData OnePoleProcessor::createParameters()
{
    addParameter(LatticeProcessorModule::Parameter("Frequency", {1, 22050, 100, 1, .5f}, Parameter::Type::Slider, true).withLabel("Hz."));
    return ParameterData(getParameters(), getNumberOfParameters());
}


void OnePoleProcessor::prepareProcessor(int sr, std::size_t)
{
	lp.reset(sr);
}

void OnePoleProcessor::process(float** buffer, std::size_t blockSize)
{
    in.resize(blockSize);

    std::copy(buffer[0], buffer[0] + blockSize, in.begin());
    auto& out = lp(in, getParameter("Frequency"));
    std::copy(out.begin(), out.end(), buffer[0]);
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
