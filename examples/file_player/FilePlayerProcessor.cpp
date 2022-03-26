
#include "FilePlayerProcessor.h"
#include <iterator>
#include <sstream>


FilePlayerProcessor::FilePlayerProcessor()
{
	
}

LatticeProcessorModule::ChannelData FilePlayerProcessor::createChannels()
{
    addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
    addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData FilePlayerProcessor::createParameters()
{
    addParameter({ "Play", {0, 1, 0, 1, 1}, "", ModuleParameter::ParamType::Switch });
    addParameter({ "Load Soundfile", {0, 1, 0, 1, 1}, "", ModuleParameter::ParamType::FileButton});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void FilePlayerProcessor::hostParameterChanged(const char* parameterID, const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Load Soundfile")
    {
        //std::cout << "File to load" << newValue;
        auto samples = getSamplesFromFile(newValue);
        soundfileSamples.resize(samples.numSamples);
        std::copy(samples.data[0], samples.data[0] + samples.numSamples, soundfileSamples.begin());
        okToDraw = true;
        fileLoaded = true;
    }
}

void FilePlayerProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Play")
    {
        isPlaying =! isPlaying;
    }
}



void FilePlayerProcessor::prepareProcessor(int /*sr*/, std::size_t /*block*/)
{

}


void FilePlayerProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    for(int i = 0; i < blockSize ; i++)
    {
        if(isPlaying && fileLoaded)
        {
            buffer[0][i] = soundfileSamples[sampleIndex];
            buffer[1][i] = soundfileSamples[sampleIndex];
            sampleIndex = sampleIndex < soundfileSamples.size() ? sampleIndex+1 : 0;
        }
        else
        {
            buffer[0][i] = 0;
            buffer[1][i] = 0;
        }
        
        
    }

}

const char* FilePlayerProcessor::getSVGXml()
{
    const int width = 200;
    const int height = 100;
    svg::Dimensions dimensions(width, height);
    svg::Document doc("rms.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
    svg::Path svgPath(svg::Fill(svg::Color("#00A2A4")), svg::Stroke(1, svg::Color("#00A2A4"), 1));

    svgPath << svg::Point(0, height/2.f);
    for (int i = 0; i < soundfileSamples.size(); i += soundfileSamples.size()/1000)
    {
        double x = remap(float(i), 0, soundfileSamples.size(), 0, width);
        double y = remap(soundfileSamples[i], -1, 1, height, 0);
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
	__declspec(dllexport) LatticeProcessorModule* create() { return new FilePlayerProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new FilePlayerProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
