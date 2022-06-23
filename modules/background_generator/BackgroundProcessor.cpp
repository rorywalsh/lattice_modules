
#include "BackgroundProcessor.h"
#include <iterator>
#include <sstream>


BackgroundProcessor::BackgroundProcessor()
: table(1000), signal(64), samples(256, 0)
{
    std::size_t n = 1;
    for (auto &s : table)
    {
      s = std::fabs(std::rand() % 100) / 100.f;
    }
    
    colours.push_back(svg::Color("#00ABD1"));
    colours.push_back(svg::Color("#0BB3BF"));
    colours.push_back(svg::Color("#00A2A4"));
    colours.push_back(svg::Color("#77C1A4"));
    
    
}

LatticeProcessorModule::ChannelData BackgroundProcessor::createChannels()
{
    addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData BackgroundProcessor::createParameters()
{
    addParameter({ "Y", {0, 600, 0, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void BackgroundProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    okToDraw = true;

}


void BackgroundProcessor::prepareProcessor(int /*sr*/, std::size_t block)
{
    signal.resize(block);
}


void BackgroundProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    //std::copy(buffer[0], buffer[0] + blockSize, signal.begin());
    
    if(frameCnt == 64)
        okToDraw = true;
    
    frameCnt = frameCnt < 64 ? frameCnt+1 : 0;
    

    samples.erase(samples.begin());
    samples.push_back(buffer[0][0]);
}

const char* BackgroundProcessor::getSVGXml()
{
    dimensions = svg::Dimensions(width, height);
    doc = svg::Document("1.svg", svg::Layout(dimensions, svg::Layout::TopLeft));

    //creates background svg..
    int colIndex = 0;
    for (int i = 0; i < 20; i++)
    {
        auto scale = std::fabs(std::rand() % 100) / 100.f;;
        double x = remap(i, 0, 20, 20, width+20);
        double y = scale*60;
        svg::Circle svgCirle(svg::Point(x, getParameter("Y")), (std::fabs(std::rand() % 100) / 100.f)*220, svg::Fill(colours[int(table[colIndex]*3)], 1-(scale)));
        doc << svgCirle;
        colIndex = colIndex < 3 ? colIndex+1 : 0;
    }
//    for (int i = 0; i < samples.size(); i += 4)
//    {
//        double x = remap(float(i), 0.f, static_cast<float>(samples.size()), -200.f, width+200.f);
//        double y = remap(samples[i], -1, 1, 0, height/2);
//        svg::Circle svgCirle(svg::Point(x, getParameter("Y")+y), samples[i]*50, svg::Fill(colours[rand() % colours.size()], 1-((std::rand() % 100) / 100.f)));
//                             doc << svgCirle;
//    }

    std::cout << svgText << std::endl;
    svgText = doc.toString();

    return svgText.c_str();
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new BackgroundProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new BackgroundProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
