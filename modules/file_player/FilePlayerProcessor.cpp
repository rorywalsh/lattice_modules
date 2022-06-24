
#include "FilePlayerProcessor.h"
#include <iterator>
#include <sstream>

#include<iostream>
#include<filesystem>

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
    addParameter({ "Play", {0, 1, 0, 1, 1},  Parameter::Type::Switch });
    addParameter({ "Oneshot Midi", {0, 1, 0, 1, 1},  Parameter::Type::Switch });
    addParameter({ "Load Soundfile", {0, 1, 0, 1, 1},  Parameter::Type::FileButton});
    addParameter({ "Playback Rate", {-2, 2, 1, 0.001, 1}});
    addParameter({ "Gain", {0, 1, .6, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void FilePlayerProcessor::hostParameterChanged(const char* parameterID, const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if (paramName == "Load Soundfile")
    {
        filename = std::filesystem::path(newValue).filename().string();
        auto samples = getSamplesFromFile(newValue);
        numSamples = samples.numSamples;
        if(numSamples == 0)
            return;
        
        releaseSegment = numSamples;
        soundfileSamples.resize(samples.numSamples);
        std::copy(samples.data[0], samples.data[0] + samples.numSamples, soundfileSamples.begin());
        okToDraw = true;
        fileLoaded = true;
    }
}

void FilePlayerProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    updateParameter(paramName, newValue);

    if(paramName == "Play")
    {
        isPlaying = (newValue == 1 ? true : false);
        if (newValue == 1)
        {
            releaseSegment = numSamples;
            sampleIndex = 0;
        }
    }
    else if (paramName == "Oneshot Midi")
    {
        if (newValue == 0)
            releaseSegment = 0;
    }
    else if (paramName == "Playback Rate")
        sampleIncrement = newValue;
    
}



void FilePlayerProcessor::prepareProcessor(int /*sr*/, std::size_t /*block*/)
{

}


void FilePlayerProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    for(int i = 0; i < blockSize ; i++)
    {
        if(isPlaying && fileLoaded && releaseSegment > 0)
        {
            buffer[0][i] = soundfileSamples[static_cast<int>(sampleIndex)] * getParameter("Gain");
            buffer[1][i] = soundfileSamples[static_cast<int>(sampleIndex)] * getParameter("Gain");
            if(sampleIncrement < 0)
                sampleIndex = sampleIndex > 0 ? sampleIndex+sampleIncrement : soundfileSamples.size() - 1;
            else
                sampleIndex = sampleIndex < soundfileSamples.size() - 1 ? sampleIndex + sampleIncrement : 0;

            if ( noteOff == true || getParameter("Oneshot Midi") == 1 )
                releaseSegment--;
        }
        else
        {
            buffer[0][i] = 0;
            buffer[1][i] = 0;
        }
    }

}

void FilePlayerProcessor::startNote(int noteNumber, float velocity)
{
    isPlaying = true;
    releaseSegment = numSamples;
    sampleIndex = 0;
    noteOff = false;
}

void FilePlayerProcessor::stopNote(int, float velocity)
{
    noteOff = true;
    if (getParameter("Oneshot Midi") == 0)
        releaseSegment = 0; //apply short fade out on release
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
    svgPath << svg::Point(width, height/2);
    doc << svgPath;
    doc << svg::Text(svg::Point(0, height-15), filename, svg::Fill(svg::Color(225, 225, 225)));
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