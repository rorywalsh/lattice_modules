
#include "BubblesProcessor.h"
#include <iterator>
#include <sstream>


BubblesProcessor::BubblesProcessor()
: samples(16, 0), xPositions(16)
{

    for ( auto& p : xPositions)
    {
        p = fabs((std::rand() % 100)) / 100.f;
    }
             
    colours.push_back(svg::Color("#00ABD1"));
    colours.push_back(svg::Color("#0BB3BF"));
    colours.push_back(svg::Color("#00A2A4"));
    colours.push_back(svg::Color("#77C1A4"));
    
    
}

LatticeProcessorModule::ChannelData BubblesProcessor::createChannels()
{
    addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData BubblesProcessor::createParameters()
{
    addParameter({ "Y", {0, 600, 0, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void BubblesProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    okToDraw = true;

}


void BubblesProcessor::prepareProcessor(int /*sr*/, std::size_t block)
{

}


void BubblesProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    //std::copy(buffer[0], buffer[0] + blockSize, signal.begin());
    
    //if(frameCnt == 32)
        okToDraw = true;
    
    //frameCnt = frameCnt < 32 ? frameCnt+1 : 0;
    

    samples.erase(samples.begin());
    samples.push_back(buffer[0][0]);
}

const char* BubblesProcessor::getSVGXml()
{
    dimensions = svg::Dimensions(width, height);
    doc = svg::Document("1.svg", svg::Layout(dimensions, svg::Layout::TopLeft));

    for (int i = 0; i < xPositions.size(); i ++)
    {
        double x = remap(i, 0.f, static_cast<float>(xPositions.size()), -100.f, width+100);
        double y = 80;//remap(smooth(samples[i], .01), -1, 1, 0, height/2);
        svg::Circle svgCirle(svg::Point(x, y), xPositions[i%8]*80, svg::Fill(colours[int(xPositions[i])*colours.size()], smooth(getRMS(samples)*3, .3)));
        doc << svgCirle;
    }

    svgText = doc.toString();

    return svgText.c_str();

}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new BubblesProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new BubblesProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
