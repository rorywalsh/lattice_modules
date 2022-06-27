
#include "GainProcessor.h"
#include <iterator>
#include <sstream>


GainProcessor::GainProcessor()
	:LatticeProcessorModule(),
	samples(512, 0)
{

}

LatticeProcessorModule::ChannelData GainProcessor::createChannels()
{
	addChannel({ "input", ChannelType::input });
	addChannel({ "gain", ChannelType::input });
	addChannel({ "output", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData GainProcessor::createParameters()
{
    addParameter({"Gain", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f), Parameter::Type::Slider, true});
    return {getParameters(), getNumberOfParameters()};
}

void GainProcessor::prepareProcessor(int sr, std::size_t block)
{
  fs = sr;
}  


void GainProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    float g = getParameter("Gain");

    for(int i = 0; i < blockSize ; i++)
      buffer[0][i] *= smooth(g, 0.01, fs) + (isInputConnected(1) ? buffer[1][i] : 0);
    samples.erase(samples.begin());
    samples.push_back(buffer[0][0]);
}

const char* GainProcessor::getSVGXml()
{
	const float width = 200;
	const float height = 180;
	svg::Dimensions dimensions(width, height);
	svg::Document doc("rms.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
	svg::Polyline svgPathLeft(svg::Fill(), svg::Stroke(1, svg::Color("#00ABD1"), 1));
  
	for (int i = 0; i < samples.size(); i += 12)
	{
		double x = remap(float(i), 0.f, static_cast<float>(samples.size()), 0.f, width);
		double y = remap(samples[i], -1, 1, 0, height/2);
        svgPathLeft << svg::Point(x, y);;
	}

	doc << svgPathLeft;
	svgText = doc.toString();
	return svgText.c_str();
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
