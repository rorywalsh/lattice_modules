
#include "TableViewerProcessor.h"
#include <iterator>
#include <sstream>


TableViewerProcessor::TableViewerProcessor()
: table(1000), signal(64)
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
    
    dimensions = svg::Dimensions(width, height);
    doc = svg::Document("1.svg", svg::Layout(dimensions, svg::Layout::TopLeft));

}

LatticeProcessorModule::ChannelData TableViewerProcessor::createChannels()
{
    addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData TableViewerProcessor::createParameters()
{
    addParameter({ "Y", {0, 600, 0, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void TableViewerProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    okToDraw = true;

}


void TableViewerProcessor::prepareProcessor(int /*sr*/, std::size_t block)
{
    signal.resize(block);
}


void TableViewerProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    std::copy(buffer[0], buffer[0] + blockSize, signal.begin());
    
    if(frameCnt == 128)
        okToDraw = true;
    
    frameCnt = frameCnt < 128 ? frameCnt+1 : 0;
}

const char* TableViewerProcessor::getSVGXml()
{

    int colIndex = 0;

    auto scale = table[xIndex];
    double x = remap(xIndex, 0, 20, -50, width+50);
    double y = scale*60;
    svg::Circle svgCirle(svg::Point(x, getParameter("Y")), table[xIndex]*100, svg::Fill(colours[int(table[colIndex]*3)], 1-(scale)));
    doc << svgCirle;
    colIndex = colIndex < 3 ? colIndex+1 : 0;


    
    float gain = smooth(getRMS(signal) * 350, .5);
    scale = table[tableIndex];
    x = remap(xIndex, 0, 30, -50, width+200);
    y = scale*60;
    svgCirle = svg::Circle(svg::Point(x, getParameter("Y")), table[tableIndex]*gain, svg::Fill(colours[rand() % colours.size()], 1-(scale)));
    doc << svgCirle;
    tableIndex = tableIndex<table.size()-1?tableIndex+1:0;
    
    xIndex = xIndex < 30 ? xIndex+1 : 0;

    //svg::Rectangle svgRect(svg::Point(0, 0), width, height, svg::Fill(svg::Color::Transparent), svg::Stroke(2, svg::Color::Blue));
    //doc << svgRect;
    svgText = doc.toString();
    
    return svgText.c_str();
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new TableViewerProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new TableViewerProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
