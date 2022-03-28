#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"
#include "Conv.h"

class TableConvProcessor : public LatticeProcessorModule
{
public:
    TableConvProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;

    /* override this method if you want to draw to the Lattice generic editor viewport */
    const char* getSVGXml() override;

    /* override this method and return true if you wish to enable drawing on the generic editor viewport */
    bool canDraw() override {
        auto draw = okToDraw;
        okToDraw = false;
        return draw;
    }

    static float remap(float value, float from1, float to1, float from2, float to2)
    {
        return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
    }
    
    const char* getModuleName() override {    return "Table Convolution";     }
    
private:
    std::vector<float> irTable;
    bool okToDraw = true;
    std::string svgText;
    Aurora::IR<float> ir;
    Aurora::Conv<float> delay;
    Aurora::Mix<float> mix;
    std::vector<float> inL, inR;
};
