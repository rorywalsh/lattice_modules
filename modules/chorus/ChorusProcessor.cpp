
#include "ChorusProcessor.h"
#include <iterator>


ChorusProcessor::ChorusProcessor()
  : chorus(Aurora::def_sr), inL(Aurora::def_vsize), inR(Aurora::def_vsize)
{
    
}

LatticeProcessorModule::ParameterData ChorusProcessor::createParameters()
{
    addParameter({ "Delay Time (L)", {0, 0.05, .017f, .0001f, 1}, Parameter::Type::Slider, true});
    addParameter({ "LFO Frequency (L)", {0, 10, .96, .001f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Delay Time (R)", {0, 0.05, .013f, .0001f, 1}, Parameter::Type::Slider, true});
    addParameter({ "LFO Frequency (R)", {0, 10, 1.28f, .001f, 1}, Parameter::Type::Slider, true });
    addParameter({ "Direct signal", {0, 1, .2f, .001f, 1}, Parameter::Type::Slider, true });
    addParameter({ "Effect signal", {0, 1, 1.f, .001f, 1}, Parameter::Type::Slider, true });
    return ParameterData(getParameters(), getNumberOfParameters());
}


void ChorusProcessor::prepareProcessor(int sr, std::size_t block)
{
	chorus.reset(sr);
	chorus.vsize(block);
}


void ChorusProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

LatticeProcessorModule::ChannelData ChorusProcessor::createChannels()
{
    addChannel({"Left Input", ChannelType::input });
    addChannel({"Right Input", ChannelType::input });
    addChannel({"Left Output", ChannelType::output });
    addChannel({"Right Output", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}

void ChorusProcessor::process(float **buffer, std::size_t blockSize)
{   
    inL.resize(blockSize);
    inR.resize(blockSize);
    chorus.vsize(blockSize);
    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());

    const float d = getParameter("Direct signal");
    const float c = getParameter("Effect signal");

    getParameter("test");

    auto &l = chorus(inL, getParameter("LFO Frequency (L)"), getParameter("Delay Time (L)"), 0);
    auto &r = chorus(inR, getParameter("LFO Frequency (R)"), getParameter("Delay Time (R)"), 1);

    for(int i = 0; i < blockSize ; i++)
    {
      buffer[0][i] = buffer[0][i]*d + (l[i] * 0.1 - r[i] * 0.13)*c;
      buffer[1][i] = buffer[1][i]*d + (r[i] * 0.1 - l[i] * 0.13)*c;
    }
    
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new ChorusProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new ChorusProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
