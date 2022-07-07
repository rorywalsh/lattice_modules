
#include "WaveformViewerProcessor.h"
#include <iterator>
#include <sstream>


WaveformViewerProcessor::WaveformViewerProcessor()
{
	samples.resize(512);
}

LatticeProcessorModule::ChannelData WaveformViewerProcessor::createChannels()
{
	addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
	addChannel({"Input 2", LatticeProcessorModule::ChannelType::input });
	addChannel({"Output 1", LatticeProcessorModule::ChannelType::output });
	addChannel({"Output 2", LatticeProcessorModule::ChannelType::output });
	return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData WaveformViewerProcessor::createParameters()
{
	addParameter({ "Gain", {0, 1, .1f, .0001f, 1} });
	return ParameterData(getParameters(), getNumberOfParameters());
}


void WaveformViewerProcessor::prepareProcessor(int /*sr*/, std::size_t /*block*/)
{

}


void WaveformViewerProcessor::process(float** buffer, std::size_t blockSize)
{
	inL.resize(blockSize);
	inR.resize(blockSize);

	std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
	std::copy(buffer[1], buffer[1] + blockSize, inR.begin());

	for (int i = 0; i < blockSize; i++)
	{
		buffer[0][i] *= getParameter("Gain");
		buffer[1][i] *= getParameter("Gain");
	}

	if (getRMS(inL) > 0)
		okToDraw = true;

	samples.erase(samples.begin());
	samples.push_back(inL[0]);
}

const char* WaveformViewerProcessor::getSVGXml()
{
	okToDraw = true;
	const int width = 200;
	const int height = 40;
	svg::Dimensions dimensions(width, height);
	svg::Document doc("rms.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
	svg::Polyline svgPath(svg::Fill(), svg::Stroke(1, svg::Color("#00ABD1"), 1));

	for (int i = 0; i < samples.size(); i += 12)
	{
		double x = remap(float(i), 0, samples.size(), 0, width);
		double y = remap(samples[i] * .6f, -1, 1, 0, height);
		auto pos = svg::Point(x, y);
		svgPath << pos;
	}


	doc << svgPath;
	svgText = doc.toString();
	return svgText.c_str();
}


// the class factories
#ifdef WIN32
    extern "C" 
    {
        __declspec(dllexport) LatticeProcessorModule* create() { return new WaveformViewerProcessor; }
    };

    extern "C" 
    {
        __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
    };
#else
    extern "C" LatticeProcessorModule* create(){     return new WaveformViewerProcessor;         }
    extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
