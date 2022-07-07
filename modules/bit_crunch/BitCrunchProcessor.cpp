
#include "BitCrunchProcessor.h"
#include <iterator>
#include <sstream>


BitCrunchProcessor::BitCrunchProcessor()
{

}

LatticeProcessorModule::ChannelData BitCrunchProcessor::createChannels()
{
    addChannel({ "Input", ChannelType::input });
    addChannel({ "Output", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData BitCrunchProcessor::createParameters()
{
    addParameter({ "Bit Depth", {2, 32, 32, 0.001f, 1.f}, Parameter::Type::Slider, true});
    addParameter({ "Sample Rate", {0, 1, 1, .0001f, 1.f}, Parameter::Type::Slider, true});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void BitCrunchProcessor::prepareProcessor(int sr, std::size_t block)
{
    sampleRate = sr;
}



void BitCrunchProcessor::process(float** buffer, std::size_t blockSize)
{
    float bitDepth = std::powf(2.f, static_cast<int>(getParameter("Bit Depth")));
    int rateReducion = (1.f - getParameter("Sample Rate")) * 50;
    std::cout << rateReducion << std::endl;
    
    for (int c = 0 ; c < getNumberOfOutputChannels() ; c++)
    {
        for(std::size_t i = 0; i < blockSize ; i++)
        {
            // REDUCE BIT DEPTH :::::
            float val = buffer[c][i];
            float remainder = fmodf(val, 1/bitDepth);
            buffer[c][i] = val - remainder;
            
            if (rateReducion > 1)
            {
                if (i % rateReducion != 0)
                    buffer[c][i] = buffer[c][i - (i % rateReducion)];
            }
        }

    }
}


// the class factories
#ifdef WIN32
extern "C"
{
    __declspec(dllexport) LatticeProcessorModule* create() { return new BitCrunchProcessor; }
};

extern "C"
{
    __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new BitCrunchProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
