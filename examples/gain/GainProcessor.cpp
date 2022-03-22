
#include "GainProcessor.h"
#include <iterator>
#include <sstream>


GainProcessor::GainProcessor()
	:LatticeProcessorModule()
{
	samples.resize(512);
}

LatticeProcessorModule::ChannelData GainProcessor::createChannels()
{
	addChannel({ "Gain Input 1", ChannelType::input });
	addChannel({ "Gain Input 2", ChannelType::input });
	addChannel({ "Gain Output 1", ChannelType::output });
	addChannel({ "Gain Output 2", ChannelType::output });
	return ChannelData(getChannels(), getNumberOfChannels());
}



LatticeProcessorModule::ParameterData GainProcessor::createParameters()
{
    addParameter({"Gain", LatticeProcessorModule::ModuleParameter::Range(0.f, 1.f, 0.4f, 0.001f, 1.f)});
	return ParameterData(getParameters(), getNumberOfParameters());
}

void GainProcessor::hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
{
//    ignoreParameters(parameterID, newValue);
}

void GainProcessor::prepareProcessor(int /*sr*/, std::size_t /*block*/)
{

}  


void GainProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    inL.resize(blockSize);
    inR.resize(blockSize);

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());

    for(int i = 0; i < blockSize ; i++)
    {
        buffer[0][i] *= getParameter("Gain");
        buffer[1][i] *= getParameter("Gain");
    }

	if (getRMS(inL) > 0)
		okToDraw = true;

	samples.erase(samples.begin());
	samples.push_back(inL[0]);
}

std::string GainProcessor::getSVGXml()
{
	okToDraw = true;
	const float width = 200;
	const float height = 80;
	svg::Dimensions dimensions(width, height);
	svg::Document doc("rms.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
	svg::Polyline svgPath(svg::Fill(), svg::Stroke(1, svg::Color("#00ABD1"), 1));

	for (int i = 0; i < samples.size(); i += 12)
	{
		double x = remap(float(i), 0.f, static_cast<float>(samples.size()), 0.f, width);
		double y = remap(samples[i] * .6f, -1, 1, 0, height);
		auto pos = svg::Point(x, y);
		svgPath << pos;
	}

	doc << svgPath;
	return doc.toString();
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new GainProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new GainProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
