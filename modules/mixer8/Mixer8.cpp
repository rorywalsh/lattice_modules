
#include "Mixer8.h"
#include <iterator>
#include <sstream>


Mixer8::Mixer8()
	:LatticeProcessorModule()
{

}

LatticeProcessorModule::ChannelData Mixer8::createChannels()
{
	addChannel({ "input1", ChannelType::input });
	addChannel({ "input2", ChannelType::input });
    addChannel({ "input3", ChannelType::input });
    addChannel({ "input4", ChannelType::input });
	addChannel({ "outputL", ChannelType::output });
    addChannel({ "outputR", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData Mixer8::createParameters()
{
    addParameter({"Gain 1", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.5f, 0.001f, 1.f), Parameter::Type::Slider, true});
    addParameter({"Pan 1", LatticeProcessorModule::Parameter::Range(-1.f, 1.f, 0.0f, 0.001f, 1.f), Parameter::Type::Slider, true});
    addParameter({"Gain 2", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.5f, 0.001f, 1.f), Parameter::Type::Slider, true});
    addParameter({"Pan 2", LatticeProcessorModule::Parameter::Range(-1.f, 1.f, 0.0f, 0.001f, 1.f), Parameter::Type::Slider, true});
    addParameter({"Gain 3", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.5f, 0.001f, 1.f), Parameter::Type::Slider, true});
    addParameter({"Pan 3", LatticeProcessorModule::Parameter::Range(-1.f, 1.f, 0.0f, 0.001f, 1.f), Parameter::Type::Slider, true});
    addParameter({"Gain 4", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.5f, 0.001f, 1.f), Parameter::Type::Slider, true});
    addParameter({"Pan 4", LatticeProcessorModule::Parameter::Range(-1.f, 1.f, 0.0f, 0.001f, 1.f), Parameter::Type::Slider, true});
    return {getParameters(), getNumberOfParameters()};
}

void Mixer8::prepareProcessor(int sr, std::size_t block)
{
  fs = sr;
}  


void Mixer8::process(float** buffer, std::size_t blockSize)
{
    float g1 = gSmooth1(getParameter("Gain 1"), 0.001);
    float g2 = gSmooth2(getParameter("Gain 2"), 0.001);
    float g3 = gSmooth3(getParameter("Gain 3"), 0.001);
    float g4 = gSmooth4(getParameter("Gain 4"), 0.001);
    float p1 = pSmooth1(((getParameter("Pan 1") + 1.f) / 2.f), 0.01f);;
    float p2 = pSmooth2(((getParameter("Pan 2") + 1.f) / 2.f), 0.01f);;
    float p3 = pSmooth3(((getParameter("Pan 3") + 1.f) / 2.f), 0.01f);;
    float p4 = pSmooth4(((getParameter("Pan 4") + 1.f) / 2.f), 0.01f);;
    
    
    for(int i = 0; i < blockSize ; i++)
    {
        float in1 = buffer[0][i];
        float in2 = buffer[1][i];
        float in3 = buffer[2][i];
        float in4 = buffer[3][i];
        
        buffer[0][i] = (1-p1)*(isInputConnected(0) ? in1 * g1 : 0);
        buffer[1][i] = (p1)*(isInputConnected(0) ? in1 * g1 : 0);
        
        buffer[0][i] += (1-p2)*(isInputConnected(1) ? in2 * g2 : 0);
        buffer[1][i] += (p2)*(isInputConnected(1) ? in2 * g2 : 0);
        
        buffer[0][i] += (1-p3)*(isInputConnected(2) ? in3 * g3 : 0);
        buffer[1][i] += (p3)*(isInputConnected(2) ? in3 * g3 : 0);
        
        buffer[0][i] += (1-p4)*(isInputConnected(3) ? in4 * g4 : 0);
        buffer[1][i] += (p4)*(isInputConnected(3) ? in4 * g4 : 0);
    }

}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new Mixer8; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new Mixer8;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
