#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"

class GainProcessor : public LatticeProcessorModule
{
public:
    GainProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;

    /* override this method if you want to draw to the Lattice generic editor viewport */
    const char* getSVGXml() override;

    /* override this method and return true if you wish to enable drawing on the generic editor viewport */
    bool canDraw() override { return true; }
    
    
    const char* getModuleName() override {    return "Gain";     }
    
private:
	static int remap(float value, float rangeMin, float rangeMax, float newRangeMin, float newRangeMax)
	{
		return static_cast<int>(newRangeMin + (value - rangeMin) * (newRangeMax - newRangeMin) / (rangeMax - rangeMin));
	}

	double getRMS(std::vector<float> const& v)
	{
		double sum = 0;
		auto const count = static_cast<float>(v.size());

		for (int i = 0; i < count; i++)
			sum += pow(v[i], 2);

		return sqrt(sum / count);
	}

	float amp = .5f;
    int numChannels = 0;
    std::vector<float> inL;
    std::vector<float> inR;
	std::vector<float> samplesL;
    std::vector<float> samplesR;
	bool okToDraw = true;
    std::string svgText;
};

