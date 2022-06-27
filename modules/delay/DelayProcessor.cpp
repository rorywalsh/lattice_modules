
#include "DelayProcessor.h"
#include <iterator>


DelayProcessor::DelayProcessor():
  delayL(2.f),
  delayR(2.f),
  inL(Aurora::def_vsize),
  inR(Aurora::def_vsize)
{
    
}


LatticeProcessorModule::ChannelData  DelayProcessor::createChannels()
{
   addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData DelayProcessor::createParameters()
{
    addParameter({ "Delay Time Left", {0, 2, .1f, .01f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Feedback Left", {0, 1, .5f, .01f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Dry Mix Left", {0, 1, .5f, .01f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Delay Time Right", {0, 2, .1f, .01f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Feedback Right", {0, 1, .5f, .01f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Dry Mix Right", {0, 1, .5f, .01f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Enabled", {0, 1, 1, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void DelayProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
   delayL.reset(2.f, sr);
   delayL.reset(2.f, sr);
   samplingRate = sr;
}

void DelayProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    inL.resize(blockSize);
    inR.resize(blockSize);  
    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());
    auto &outL = delayL(inL, timeL(getParameter("Delay Time Left"), 0.01),
			getParameter("Feedback Left"), getParameter("Dry Mix Left"));
    auto &outR = delayR(inR, timeR(getParameter("Delay Time Right"), 0.01),
			getParameter("Feedback Right"), getParameter("Dry Mix Right"));
    std::copy(outL.begin(), outL.end(), buffer[0]);
    std::copy(outR.begin(), outR.end(), buffer[1]);
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new DelayProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new DelayProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
