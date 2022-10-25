
#include "Mixer16.h"
#include <iterator>
#include <sstream>


Mixer16::Mixer16()
	:LatticeProcessorModule()
{
    for (int i = 0; i < SIZE; i++)
        gainParams.push_back("Gain " + std::to_string(i+1));
    for (int i = 0; i < SIZE; i++)
        panParams.push_back("Pan " + std::to_string(i + 1));
}

LatticeProcessorModule::ChannelData Mixer16::createChannels()
{
	addChannel({ "input1", ChannelType::input });
	addChannel({ "input2", ChannelType::input });
    addChannel({ "input3", ChannelType::input });
    addChannel({ "input4", ChannelType::input });
    addChannel({ "input5", ChannelType::input });
    addChannel({ "input6", ChannelType::input });
    addChannel({ "input7", ChannelType::input });
    addChannel({ "input8", ChannelType::input });
    addChannel({ "input9", ChannelType::input });
    addChannel({ "input10", ChannelType::input });
    addChannel({ "input11", ChannelType::input });
    addChannel({ "input12", ChannelType::input });
    addChannel({ "input13", ChannelType::input });
    addChannel({ "input14", ChannelType::input });
    addChannel({ "input15", ChannelType::input });
    addChannel({ "input16", ChannelType::input });
	addChannel({ "outputL", ChannelType::output });
    addChannel({ "outputR", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData Mixer16::createParameters()
{
    for (int i = 0; i < SIZE; i++)
    {
        addParameter({ gainParams[i].c_str(), LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.5f, 0.001f, 1.f), Parameter::Type::Slider, true});
        addParameter({ panParams[i].c_str(), LatticeProcessorModule::Parameter::Range(-1.f, 1.f, 0.0f, 0.001f, 1.f), Parameter::Type::Slider, true});
    }


    return {getParameters(), getNumberOfParameters()};
}

void Mixer16::prepareProcessor(int sr, std::size_t block)
{
  fs = sr;
}  


void Mixer16::process(float** buffer, std::size_t blockSize)
{
    float gain[SIZE];
    float pan[SIZE];

    for (int i = 0; i < SIZE; i++)
    {
        gain[i] = gainSmooth[i](getParameter(gainParams[i].c_str()), 0.001);
        pan[i] = panSmooth[i]((getParameter(panParams[i].c_str())+1.f / 2.f), 0.001);
    }

    
    for(int i = 0; i < blockSize ; i++)
    {
        float ins[SIZE] = {};
        for( int p = 0 ; p < SIZE ; p++ )
            ins[p] = buffer[p][i];
        
        buffer[0][i] = (1 - pan[0]) * (isInputConnected(0) ? ins[0] * gain[0] : 0);
        buffer[1][i] = (pan[0]) * (isInputConnected(0) ? ins[0] * gain[0] : 0);

        for (int x = 1; x < SIZE; x++)
        {
            buffer[0][i] += (1 - pan[x]) * (isInputConnected(i) ? ins[x] * gain[x] : 0);
            buffer[1][i] += (pan[x]) * (isInputConnected(i) ? ins[x] * gain[x] : 0);
        }
    }

}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new Mixer16; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new Mixer16;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
