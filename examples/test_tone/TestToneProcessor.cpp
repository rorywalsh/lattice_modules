
#include "TestToneProcessor.h"
#include <iterator>
#include <sstream>


TestToneProcessor::TestToneProcessor()
: wave(4096), 
oscL(&wave, 44100),
oscR(&wave, 44100)
{
	std::size_t n = 0;
	for (auto& s : wave) {
		s = std::sin((Aurora::twopi / wave.size()) * n++);
	}
}

LatticeProcessorModule::ChannelData TestToneProcessor::createChannels()
{
    addChannel({ "Input 1", ChannelType::input });
    addChannel({ "Input 2", ChannelType::input });
    addChannel({ "Output 2", ChannelType::output });
    addChannel({ "Output 2", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData TestToneProcessor::createParameters()
{
    addParameter({ "Frequency", {0, 22050, 1000.f, 1.f, .5f}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void TestToneProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
	oscL.reset(sr);
}


void TestToneProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData /*hostInfo*/)
{

	oscL.vsize(blockSize);
	oscR.vsize(blockSize);
	const std::vector<float>& outL = oscL(.5f, getParameter("Frequency"));
	const std::vector<float>& outR = oscR(.5f, getParameter("Frequency"));

    for(std::size_t i = 0; i < blockSize ; i++)
    {
		buffer[0][i] = outL[i];
		buffer[1][i] = outR[i];
    }
}
