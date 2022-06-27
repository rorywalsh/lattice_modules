
#include "VibratoProcessor.h"
#include <iterator>


VibratoProcessor::VibratoProcessor()
  : vibr(Aurora::def_sr), in(Aurora::def_vsize)
{
    
}

LatticeProcessorModule::ParameterData VibratoProcessor::createParameters()
{
    addParameter({ "Depth", {0, 1, 0, .001f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Rate", {0, 20, 7, .001f, 1}, Parameter::Type::Slider, true});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void VibratoProcessor::prepareProcessor(int sr, std::size_t block)
{
	vibr.reset(sr);
}


void VibratoProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

LatticeProcessorModule::ChannelData VibratoProcessor::createChannels()
{
    addChannel({"Input", ChannelType::input });
    addChannel({"Output", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}

void VibratoProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData)
{
   
    in.resize(blockSize);
    std::copy(buffer[0], buffer[0] + blockSize, in.begin());
    auto &s = vibr(in, getParameter("Rate"), getParameter("Depth"));
    std::copy(s.begin(),s.end(),buffer[0]);
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new VibratoProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new VibratoProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
