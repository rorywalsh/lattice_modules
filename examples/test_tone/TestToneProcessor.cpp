
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
    addParameter({ "Frequency", {0, 22050, 1000.f, 1.f, .5f}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void TestToneProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
	osc.reset(sr);
}


void TestToneProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData /*hostInfo*/)
{

	osc.vsize(blockSize);
	const std::vector<float>& out = osc(.5f, getParameter("Frequency"));

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
