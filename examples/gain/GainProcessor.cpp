
#include "GainProcessor.h"
#include <iterator>
#include <sstream>


GainProcessor::GainProcessor()
	:LatticeProcessorModule(),
	samplesL(512, 0),
	samplesR(512, 0)
{

}

LatticeProcessorModule::ChannelData GainProcessor::createChannels()
{
	addChannel({ "Gain Input 1", ChannelType::input });
	addChannel({ "Gain Input 2", ChannelType::input });
	addChannel({ "Gain Output 1", ChannelType::output });
	addChannel({ "Gain Output 2", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData GainProcessor::createParameters()
{
    addParameter({"Gain Left", LatticeProcessorModule::ModuleParameter::Range(0.f, 1.f, 0.4f, 0.001f, 1.f)});
    addParameter({"Gain Right", LatticeProcessorModule::ModuleParameter::Range(0.f, 1.f, 0.4f, 0.001f, 1.f)});
	return {getParameters(), getNumberOfParameters()};
}

void GainProcessor::prepareProcessor(int /*sr*/, std::size_t block)
{
//    samplesL.resize(block);
//    samplesR.resize(block);
}  


void GainProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{

    for(int i = 0; i < blockSize ; i++)
    {
        buffer[0][i] *= getParameter("Gain Left");
        buffer[1][i] *= getParameter("Gain Right");
    }

	samplesL.erase(samplesL.begin());
	samplesL.push_back(buffer[0][0]);
    samplesR.erase(samplesR.begin());
    samplesR.push_back(buffer[1][0]);
}

const char* GainProcessor::getSVGXml()
{
	const float width = 200;
	const float height = 180;
	svg::Dimensions dimensions(width, height);
	svg::Document doc("rms.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
	svg::Polyline svgPathLeft(svg::Fill(), svg::Stroke(1, svg::Color("#00ABD1"), 1));
    svg::Polyline svgPathRight(svg::Fill(), svg::Stroke(1, svg::Color("#00ABD1"), 1));

	for (int i = 0; i < samplesL.size(); i += 12)
	{
		double x = remap(float(i), 0.f, static_cast<float>(samplesL.size()), 0.f, width);
		double yL = remap(samplesL[i], -1, 1, 0, height/2);
        svgPathLeft << svg::Point(x, yL);;
        double yR = remap(samplesR[i], -1, 1, height/2, height);
        svgPathRight << svg::Point(x, yR);;
	}

	doc << svgPathLeft;
    doc << svgPathRight;
    
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
