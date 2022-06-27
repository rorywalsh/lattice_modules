
#include "EqProcessor.h"
#include <iterator>
#include <sstream>


EqProcessor::EqProcessor():eq(44100)
{

}

LatticeProcessorModule::ChannelData EqProcessor::createChannels()
{
    addChannel({ "Input", LatticeProcessorModule::ChannelType::input });
    addChannel({"Output" , LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData EqProcessor::createParameters()
{
    addParameter({ "Frequency", {1, 22050, 100, 1, .5f}, Parameter::Type::Slider, true});
	addParameter({ "Bandwidth", {1, 22050, 100, 1, .5f}, Parameter::Type::Slider, true});
	addParameter({ "Output Gain", {0, 2.f, 0.5f, 0.01f, 1}, Parameter::Type::Slider, true});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void EqProcessor::prepareProcessor(int sr, std::size_t)
{
	eq.reset(sr);}


void EqProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData)
{
    in.resize(blockSize);
    
    std::copy(buffer[0], buffer[0] + blockSize, in.begin());

	auto& out = eq(in, getParameter("Output Gain"), getParameter("Frequency"), getParameter("Bandwidth"));

    for (int i = 0; i < blockSize; i++)
        for (int chan = 0; chan < numChannels; chan++)
            buffer[chan][i] = out[i];
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
