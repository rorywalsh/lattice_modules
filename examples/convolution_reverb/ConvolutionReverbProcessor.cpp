
#include "ConvolutionReverbProcessor.h"
#include <iterator>
#include <sstream>


ConvolutionReverbProcessor::ConvolutionReverbProcessor()
  : irTable(Aurora::def_sr), delay(create_reverb(irTable)), inL(Aurora::def_vsize), inR(Aurora::def_vsize)
{
	
}

LatticeProcessorModule::ChannelData ConvolutionReverbProcessor::createChannels()
{
    addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
    addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData ConvolutionReverbProcessor::createParameters()
{
    addParameter({ "Reverb Gain", {0, 1.f, .5f, .01f, 1}});
    addParameter({ "Bypass", {0, 1, 0, 1, 1}, "", ModuleParameter::ParamType::Switch});
    addParameter({ "Load IR", {0, 1, 0, 1, 1}, "", ModuleParameter::ParamType::FileButton});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void ConvolutionReverbProcessor::hostParameterChanged(const char* parameterID, const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Load IR")
    {
        //std::cout << "File to load" << newValue;
        auto samples = getSamplesFromFile(newValue);
        irTable.resize(samples.numSamples);	
        std::copy(samples.data[0], samples.data[0] + samples.numSamples, irTable.begin());
        okToDraw = true;
        reset_reverb(delay,irTable);
    }
}

void ConvolutionReverbProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Bypass")
    {
        bypass = newValue == 1 ? true : false;
    }
}



void ConvolutionReverbProcessor::prepareProcessor(int /*sr*/, std::size_t /*block*/)
{

}


void ConvolutionReverbProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    
    inL.resize(blockSize);
    inR.resize(blockSize);
    
    for(int n = 0; n < blockSize; n++)
      inL[n] = (buffer[0][n] + buffer[1][n])*0.5;
    

    auto &outL = mix(delay(inL, getParameter("Reverb Gain")), inL);

    for(int i = 0; i < blockSize ; i++)
    {
        if(bypass)
        {
            buffer[0][i] = buffer[0][i];
            buffer[1][i] = buffer[1][i];
        }
        else
        {
            buffer[0][i] = outL[i];
            buffer[1][i] = outL[i];
        }

    }

}

const char* ConvolutionReverbProcessor::getSVGXml()
{
    const int width = 200;
    const int height = 100;
    svg::Dimensions dimensions(width, height);
    svg::Document doc("rms.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
    svg::Path svgPath(svg::Fill(svg::Color("#00A2A4")), svg::Stroke(1, svg::Color("#00A2A4"), 1));

    svgPath << svg::Point(0, height/2.f);
    for (int i = 0; i < irTable.size(); i += irTable.size()/1000)
    {
        double x = remap(float(i), 0, irTable.size(), 0, width);
        double y = remap(irTable[i], -1, 1, height, 0);
        svgPath << svg::Point(x, y);
    }

    svgPath << svg::Point(width, height/2);
    svgPath << svg::Point(0, height/2);
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
extern "C" LatticeProcessorModule* create(){             return new ConvolutionReverbProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
