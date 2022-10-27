
#include "Mixer4.h"
#include <iterator>
#include <sstream>


Mixer4::Mixer4()
	:LatticeProcessorModule()
{
    for (int i = 0; i < SIZE; i++)
    {
        gainParams.push_back("Gain " + std::to_string(i + 1));
        panParams.push_back("Pan " + std::to_string(i + 1));
        inputs.push_back("input" + std::to_string(i + 1));
    }    
}

LatticeProcessorModule::ChannelData Mixer4::createChannels()
{
    for (int i = 0; i < SIZE; i++)
    {
        addChannel({ inputs[i].c_str(), ChannelType::input });
    }

	addChannel({ "outputL", ChannelType::output });
    addChannel({ "outputR", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData Mixer4::createParameters()
{
    for (int i = 0; i < SIZE; i++)
    {
        addParameter({ gainParams[i].c_str(), LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.5f, 0.001f, 1.f), Parameter::Type::Slider, true});
        addParameter({ panParams[i].c_str(), LatticeProcessorModule::Parameter::Range(-1.f, 1.f, 0.0f, 0.001f, 1.f), Parameter::Type::Slider, true});
    }


    return {getParameters(), getNumberOfParameters()};
}

void Mixer4::prepareProcessor(int sr, std::size_t block)
{
  fs = sr;
}  


void Mixer4::process(float** buffer, std::size_t blockSize)
{
    float gain[SIZE];   // holds our gain values
    float pan[SIZE];    // holds pan values

    for (int i = 0; i < SIZE; i++)
    {
        gain[i] = gainSmooth[i](getParameter(gainParams[i].c_str()), 0.001);
        pan[i] = panSmooth[i]((getParameter(panParams[i].c_str())+1.f / 2.f), 0.001);
    }

    float ins[SIZE] = {}; //temporary input buffer
    
    for(int i = 0; i < blockSize ; i++)
    {        
        for( int p = 0 ; p < SIZE ; p++ )
            ins[p] = buffer[p][i]; //assign to ins[] as buffer will be overwritten
        
        buffer[0][i] = (1 - pan[0]) * (isInputConnected(0) ? ins[0] * gain[0] : 0);
        buffer[1][i] = (pan[0]) * (isInputConnected(0) ? ins[0] * gain[0] : 0);

        /* doing it manually for each channel works fine 
        buffer[0][i] += (1 - pan[1]) * (isInputConnected(1) ? ins[1] * gain[1] : 0);
        buffer[1][i] += (pan[1]) * (isInputConnected(1) ? ins[1] * gain[1] : 0);   
         */

        /* this causes unwanted artefacts in the output signal for channerls 2, 3, and 4*/
        for (int x = 1; x < SIZE; x++)
        {
            buffer[0][i] += (1 - pan[x]) * (isInputConnected(x) ? ins[x] * gain[x] : 0);
            buffer[1][i] += (pan[x]) * (isInputConnected(x) ? ins[x] * gain[x] : 0);
        }
    }
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new Mixer4; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new Mixer4;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
