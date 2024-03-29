
#include "SimpleSynthProcessor.h"
#include <iterator>


SimpleSynthProcessor::Synth::Synth(float rt, float sr)
: att(0.1f), dec(0.3f), sus(0.7f), rel(rt),
squareWave(Aurora::SQUARE),
triangleWave(Aurora::TRIANGLE),
sawWave(Aurora::SAW),
env(Aurora::ads_gen(att, dec, sus), rt, Aurora::def_sr),
osc(&sawWave,sr)
{

}

void SimpleSynthProcessor::Synth::setWaveform(int waveForm)
{
    switch(waveForm)
    {
        case 1:
            osc.waveset(&sawWave);
            return;
        case 2:
            osc.waveset(&squareWave);
            return;
        case 3:
            osc.waveset(&triangleWave);
            return;
    }
}

void SimpleSynthProcessor::Synth::setSampleRate(int sr)
{
	osc.reset(sr);
	env.reset(sr);
	sawWave.reset(Aurora::SAW, sr);
        triangleWave.reset(Aurora::TRIANGLE, sr);
        squareWave.reset(Aurora::SQUARE, sr);
}

void SimpleSynthProcessor::Synth::setBlockSize(std::size_t blockSize)
{
    osc.vsize(blockSize);
    env.vsize(blockSize);
}

//======================================================================================
SimpleSynthProcessor::SimpleSynthProcessor()
:synth(.1, 44100)
{
    
}

LatticeProcessorModule::ChannelData SimpleSynthProcessor::createChannels()
{
    addChannel({"Output", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData SimpleSynthProcessor::createParameters()
{
    addParameter({"Wave", {1, 3, 1, 1, 1}, Parameter::Type::Slider, true});
    addParameter({"Attack", {0, 1, synth.getAttack(), 0.001, 1}});
    addParameter({"Decay", {0, 2, synth.getDecay(), 0.001, 1}});
    addParameter({"Sustain", {0, 1, synth.getSustain(), 0.001, 1}});
    addParameter({"Release", {0, 3, 0.1, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}

void SimpleSynthProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Wave")
    {
        synth.setWaveform(int(newValue));
    }
    else if(paramName == "Attack")
    {
        synth.setAttack(newValue);
    }
    else if(paramName == "Decay")
    {
        synth.setDecay(newValue);
    }
    else if(paramName == "Sustain")
    {
        synth.setSustain(newValue);
    }
    else if(paramName == "Release")
    {
        synth.setRelease(newValue);
    }    
}

void SimpleSynthProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
	synth.setBlockSize(blockSize);
}

void SimpleSynthProcessor::startNote(int midiNoteNumber, float velocity)
{
    setMidiNoteNumber(midiNoteNumber);
    vel = velocity;
    isNoteOn = true;
}

void SimpleSynthProcessor::stopNote (int, float /* velocity */)
{
    isNoteOn = false;
}


void SimpleSynthProcessor::processSynthVoice(float** buffer, std::size_t blockSize)
{
    const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
    synth.setBlockSize(blockSize);
    const std::vector<float> &out = synth(vel, freq, isNoteOn);
    std::copy(out.begin(),out.end(), buffer[0]);    
    
}

// the class factories
#ifdef WIN32
    extern "C" 
    {
        __declspec(dllexport) LatticeProcessorModule* create() { return new SimpleSynthProcessor; }
    };

    extern "C" 
    {
        __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
    };
#else
    extern "C" LatticeProcessorModule* create(){     return new SimpleSynthProcessor;         }
    extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
