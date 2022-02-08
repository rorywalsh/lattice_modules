
#include "WaveformViewerProcessor.h"
#include <iterator>
#include <sstream>


WaveformViewerProcessor::WaveformViewerProcessor()
{
	samples.resize(512);
}

void WaveformViewerProcessor::createChannelLayout(std::vector<std::string>& inputs, std::vector<std::string>& outputs)
{
	inputs.push_back("Input 1");
	inputs.push_back("Input 2");
	outputs.push_back("Output 1");
	outputs.push_back("Output 2");
}


void WaveformViewerProcessor::createParameters(std::vector<ModuleParameter>& parameters)
{
	parameters.push_back({ "Gain", {0, 1, .1f, .0001f, 1} });
}

void WaveformViewerProcessor::hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
{
	//    ignoreParameters(parameterID, newValue);
}

void WaveformViewerProcessor::prepareProcessor(int /*sr*/, int /*block*/)
{

}


void WaveformViewerProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
	updateParameter(parameterID, newValue);
}


void WaveformViewerProcessor::process(float** buffer, int /*numChannels*/, int blockSize,const HostInfo hostInfo)
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

std::string WaveformViewerProcessor::getSVGXml()
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
	return doc.toString();
}
