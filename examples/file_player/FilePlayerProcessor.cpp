
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
    addParameter({ "Play", {0, 1, 0, 1, 1}, ModuleParameter::ParamType::Switch });
    addParameter({ "Oneshot MIDI", {0, 1, 0, 1, 1}, ModuleParameter::ParamType::Switch });
    addParameter({ "Load Soundfile", {0, 1, 0, 1, 1}, ModuleParameter::ParamType::FileButton});
    addParameter({ "Playback Rate", {0, 2, 1, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void FilePlayerProcessor::hostParameterChanged(const char* parameterID, const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if (paramName == "Load Soundfile")
    {
        auto samples = getSamplesFromFile(newValue);
        numSamples = samples.numSamples;
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
        isPlaying =! isPlaying;
        if (!isPlaying)
            sampleIndex = 0;
    }
    else if (paramName == "Oneshot MIDI")
    {
        if (newValue == 0 && isPlaying == getParameter("Play") == 0)
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
            buffer[0][i] = soundfileSamples[static_cast<int>(sampleIndex)];
            buffer[1][i] = soundfileSamples[static_cast<int>(sampleIndex)];
            sampleIndex = sampleIndex < soundfileSamples.size()-1 ? sampleIndex+sampleIncrement : 0;
            if ( noteOff == true || getParameter("Oneshot MIDI") == 1 )
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

void FilePlayerProcessor::stopNote(float velocity)
{
    noteOff = true;
    if (getParameter("Oneshot MIDI") == 0)
        releaseSegment = 0;
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
