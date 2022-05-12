
#include "TestToneProcessor.h"
#include <iterator>
#include <sstream>


TestToneProcessor::TestToneProcessor()
: wave(4096), 
osc(&wave, 44100)
{
	std::size_t n = 0;
	for (auto& s : wave) {
		s = std::sin((Aurora::twopi / wave.size()) * n++);
	}
}

LatticeProcessorModule::ChannelData TestToneProcessor::createChannels()
{
    addChannel({ "Output", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData TestToneProcessor::createParameters()
{
    addParameter({ "Amplitude", {0, 1, 0.5f, 0.001f, 1.f}});
    addParameter({ "Frequency", {0, 1000, 100.f, 0.001f, .5f}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void TestToneProcessor::prepareProcessor(int sr, std::size_t block)
{
	osc.reset(sr);
    ampVector.resize(block);
    freqVector.resize(block);
    
}


void TestToneProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    updateParameter(paramName, newValue);
}


void TestToneProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData /*hostInfo*/)
{
    osc.vsize(blockSize);
    std::cout << getParameter("Frequency") << std::endl;
    auto& out = osc(getParameter("Amplitude"), getParameter("Frequency"));
    
    for(std::size_t i = 0; i < blockSize ; i++)
    {
        buffer[0][i] = out[i];
    }
}


// the class factories
#ifdef WIN32
extern "C"
{
    __declspec(dllexport) LatticeProcessorModule* create() { return new TestToneProcessor; }
};

extern "C"
{
    __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new TestToneProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
