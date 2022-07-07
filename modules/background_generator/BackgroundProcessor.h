#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"

class BackgroundProcessor : public LatticeProcessorModule
{
public:
    BackgroundProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, std::size_t blockSize) override;

    /* override this method if you want to draw to the Lattice generic editor viewport */
    const char* getSVGXml() override;

    /* override this method and return true if you wish to enable drawing on the generic editor viewport */
    bool canDraw() override {
        auto draw = okToDraw;
        okToDraw = false;
        return draw;
    }

    double getRMS(std::vector<float> const& v)
    {
        double sum = 0;
        auto const count = static_cast<float>(v.size());

        for (int i = 0; i < count; i++)
            sum += pow(v[i], 2);

        return sqrt(sum / count);
    }
    
    static float remap(float value, float from1, float to1, float from2, float to2)
    {
        return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
    }
    
    const char* getModuleName() override {    return "Table Viewer";     }
    
private:
    std::vector<float> table;
    std::vector<float> signal;
    std::vector<float> samples;
    bool okToDraw = true;
    std::string svgText;
    std::vector<svg::Color> colours;
    int tableIndex = 0;
    int sampleIndex = 0;
    ParamSmooth smooth;
    int frameCnt = 0;
    const int width = 400;
    const int height = 200;
    svg::Dimensions dimensions;
    svg::Document doc;
    int xIndex = 0;

};

