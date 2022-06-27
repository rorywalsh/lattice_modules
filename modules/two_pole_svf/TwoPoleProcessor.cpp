
#include "TwoPoleProcessor.h"
#include <iterator>
#include <sstream>


TwoPoleProcessor::TwoPoleProcessor():lp(44100)
{

}

LatticeProcessorModule::ChannelData TwoPoleProcessor::createChannels()
{
   addChannel({"Input", LatticeProcessorModule::ChannelType::input });
   addChannel({"Output", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData TwoPoleProcessor::createParameters()
{
    addParameter({ "Frequency", {1, 22050, 100, 1, .5f}, Parameter::Type::Slider, true});
    addParameter({ "Q", {0.5, 100, 1, 0.001f, 1.f}, Parameter::Type::Slider, true});
    addParameter({ "Filter Type", {0, 2.f, 0.f, 0.001f, 1.f}, Parameter::Type::Slider, true});
    addParameter({ "Overdrive", {0, 1, 0, 0.001f, 1.f}, Parameter::Type::Slider, true});
    return ParameterData(getParameters(), getNumberOfParameters());	
}


void TwoPoleProcessor::prepareProcessor(int sr, std::size_t block)
{
  	lp.reset(sr);
}


void TwoPoleProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData /*hostInfo*/)
{
    in.resize(blockSize);
    std::copy(buffer[0], buffer[0] + blockSize, in.begin());
    auto& out = lp(in, getParameter("Frequency"), 1.f/getParameter("Q"),
		   4*getParameter("Overdrive")-1, getParameter("Filter Type"));
    std::copy(out.begin(),out.end(),buffer[0]);
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
