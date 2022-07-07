
#include "VibratoProcessor.h"
#include <iterator>


VibratoProcessor::VibratoProcessor()
  : vibr(Aurora::def_sr), in(Aurora::def_vsize)
{
    
}

LatticeProcessorModule::ParameterData VibratoProcessor::createParameters()
{
    addParameter({ "Speed", {0, 1, 0, 0.001, 1}, Parameter::Type::Slider, true});
    addParameter({ "AM", {0, 1, 0.1, 0.001, 1}, Parameter::Type::Slider, true});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void VibratoProcessor::prepareProcessor(int sr, std::size_t block)
{
	vibr.reset(sr);
	fs = sr;
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

void VibratoProcessor::process(float** buffer, std::size_t blockSize)
{
   
    in.resize(blockSize);
    std::copy(buffer[0], buffer[0] + blockSize, in.begin());
    const float speed = 0.4 + getParameter("Speed")*3;
    
    auto &s = vibr(in,sm(speed,0.5,fs/blockSize),getParameter("AM"));
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
