
#include "TableConvProcessor.h"
#include <iterator>
#include <sstream>


TableConvProcessor::TableConvProcessor()
: irTable(Aurora::def_sr), ir(irTable, Aurora::def_vsize), delay(&ir), inL(Aurora::def_vsize), inR(Aurora::def_vsize)
{
    std::size_t n = 1;
    for (auto &s : irTable)
    {
      s = 1-((float)n/(float)irTable.size());
        n++;
    }

}

LatticeProcessorModule::ChannelData TableConvProcessor::createChannels()
{
    addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
    addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData TableConvProcessor::createParameters()
{
    addParameter({ "Gain", {0, 1, 0, 0.001, 1}});
    addParameter({ "Update Table", {0, 1, 0, 1, 1},  Parameter::Type::Trigger });
    return ParameterData(getParameters(), getNumberOfParameters());
}


void TableConvProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Update Table")
    {
        std::size_t n = 1;
        for (auto &s : irTable)
        {
          s = 1-((float)n/(float)irTable.size());
            n++;
        }
        okToDraw = true;
    }
}


void TableConvProcessor::prepareProcessor(int /*sr*/, std::size_t /*block*/)
{

}


void TableConvProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    inL.resize(blockSize);
    inR.resize(blockSize);

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());
    
    auto &outL = mix(delay(inL, 0.001), inL);
    auto &outR = mix(delay(inR, 0.001), inL);

    for(int i = 0; i < blockSize ; i++)
    {
        buffer[0][i] = outL[i];
        buffer[1][i] = outR[i];
    }

}

const char* TableConvProcessor::getSVGXml()
{
    const int width = 200;
    const int height = 80;
    svg::Dimensions dimensions(width, height);
    svg::Document doc("rms.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
    svg::Path svgPath(svg::Fill(svg::Color("#00A2A4")), svg::Stroke());

    svgPath << svg::Point(0, height);
    for (int i = 0; i < irTable.size(); i += 12)
    {
        double x = remap(float(i), 0, irTable.size(), 0, width);
        double y = remap(irTable[i] * .6f, 0, 1, height, 0);
        svgPath << svg::Point(x, y);
    }

    svgPath << svg::Point(width, height);
    svgPath << svg::Point(0, height);
    doc << svgPath;
    svgText = doc.toString();
    return svgText.c_str();
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new TableConvProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new TableConvProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
