#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"

class WaveformViewerProcessor : public LatticeProcessorModule
{
public:
	WaveformViewerProcessor();

	ChannelData createChannels() override;

	/* This function is called by he host to populate the parameter vector */
	ParameterData createParameters() override;

	/*  This function is called by the host before playback/performance */
	void prepareProcessor(int sr, std::size_t block) override;

	/* Call this method to trigger host callback */
	void triggerParameterUpdate(const std::string& parameterID, float newValue);

	/*  Main processing function called continuously by the host on the audio thread.
		paramValues is a list of parameter values passed from the host in order of their creation */
	void process(float** buffer, std::size_t blockSize) override;


	/* override this method if you want to draw to the Lattice generic editor viewport */
	const char* getSVGXml() override;

	/* override this method and return true if you wish to enable drawing on the generic editor viewport */
	bool canDraw() override { return okToDraw; }

	const char* getModuleName() override { return "SVG Waveform Viewer"; }

	static int remap(float value, float rangeMin, float rangeMax, float newRangeMin, float newRangeMax)
	{
		return newRangeMin + (value - rangeMin) * (newRangeMax - newRangeMin) / (rangeMax - rangeMin);
	}

	double getRMS(std::vector<float> const& v)
	{
		double sum = 0;
		auto const count = static_cast<float>(v.size());

		for (int i = 0; i < count; i++)
			sum += pow(v[i], 2);

		return sqrt(sum / count);
	}


private:
	float amp = .5f;
	std::vector<float> inL;
	std::vector<float> inR;
	std::vector<float> samples;
	bool okToDraw = true;
	std::string svgText;
};

