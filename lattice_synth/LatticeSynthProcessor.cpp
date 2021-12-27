
#include "LatticeSynthProcessor.h"
#include <iterator>


LatticeSynthProcessor::Synth::Synth(float rt, float sr)
: att(0.1f), dec(0.1f), sus(0.8f),
squareWave(Aurora::SQUARE),
triangleWave(Aurora::TRIANGLE),
sawWave(Aurora::SAW),
sineWave(Aurora::def_ftlen),
env(Aurora::ads_gen(att, dec, sus), rt, Aurora::def_sr),
sinOsc(&sineWave,sr),
blOsc(&sawWave, sr),
sawOscA(&sawWave, sr),
sawOscB(&sawWave, sr),
square(),
pwmToneFilter(sr),
pwmChanges(1),
fourPoleFilter(sr),
twoPoleFilter(sr)
{
    pwmChanges[0] = 0.5;
    std::size_t n = 0;
    for (auto &s : sineWave)
    {
      s = static_cast<float>(std::sin((Aurora::twopi / sineWave.size()) * n++));
    }
}

const std::vector<float>& LatticeSynthProcessor::Synth::operator()(float a, float f, bool gate)
{
    if(currentWave == 1)
    {
        return env(sinOsc(a, f), gate);
    }
    else if(currentWave == 3)
    {
        auto &pwmSmooth = pwmToneFilter(pwmChanges, 10.f);
        float off = a*(2*pwmSmooth[0] - 1.f);
        auto &sq1 = square(square(sawOscA(a, f, pwmSmooth[0]), sawOscB(-a, f)), off);
        return fourPoleFilter(env(sq1, gate), 15000, .5);
    }
    else
    {
        return fourPoleFilter(env(blOsc(a, f), gate), 15000, .5);
    }
}

void LatticeSynthProcessor::Synth::setWaveform(int waveForm)
{
    currentWave = waveForm;
    switch(waveForm)
    {
        case 2:
            blOsc.waveset(&sawWave);
            return;
        case 3:
            blOsc.waveset(&squareWave);
            return;
        case 4:
            blOsc.waveset(&triangleWave);
            return;
    }
}

void LatticeSynthProcessor::Synth::setBlockSize(int blockSize)
{
    //fourPoleFilter.vsize(blockSize);
    //twoPoleFilter.vsize(blockSize);
    pwmToneFilter.vsize(1);
    sawOscA.vsize(blockSize);
    sawOscB.vsize(blockSize);
    sinOsc.vsize(blockSize);
    blOsc.vsize(blockSize);
    env.vsize(blockSize);
}

//======================================================================================
LatticeSynthProcessor::LatticeSynthProcessor()
:vco1(.1, 44100), vco2(.1, 44100)
{
    
}


void LatticeSynthProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}

void LatticeSynthProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({"AmpAtt", {0, 1, 0.001, 0.001, 1}});
    parameters.push_back({"AmpDec", {0, 2, 0.1, 0.001, 1}});
    parameters.push_back({"AmpSus", {0, 1, 0.8, 0.001, 1}});
    parameters.push_back({"AmpRel", {0, 3, 0.1, 0.001, 1}});

    parameters.push_back({"Wave1", {1, 4, 1, 1, 1}});
    parameters.push_back({"PWM1", {0.01, .999, .5, 0.001, 1}});
    parameters.push_back({"Wave2", {1, 4, 1, 1, 1}});
    parameters.push_back({"PWM2", {0.01, .999, .5, 0.001, 1}});
    parameters.push_back({"Fine-tune", {.9, 1.1, 1, 0.001, 1}});
    parameters.push_back({"Semitones", {-12, 12, 0, 1, 1}});
    parameters.push_back({"Mix", {0, 1, 0.5, 0.001, 1}});
    
    parameters.push_back({"FilterFreq", {1, 22050, 5000, 1, 1}});
    parameters.push_back({"FilterRes", {0, 1, 0, 0.001, 1}});
}

void LatticeSynthProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
    auto parameterName = getParameterNameFromId(parameterID);
    
    if(parameterName == "Wave1")
        vco1.setWaveform(int(newValue));
    if(parameterName == "Wave2")
        vco2.setWaveform(int(newValue));
    else if(parameterName == "AmpAtt")
        vco1.setAttack(newValue);
    else if(parameterName == "AmpDec")
        vco1.setDecay(newValue);
    else if(parameterName == "AmpSus")
        vco1.setSustain(newValue);
    else if(parameterName == "AmpRel")
        vco1.setRelease(newValue);
    else if(parameterName == "Fine-tune")
        vco1.setDetune(newValue);
    else if(parameterName == "Mix")
        mix = newValue;
    else if(parameterName == "Semitones")
        vco1.setSemitones(newValue);
    else if(parameterName == "PWM1")
        vco1.setPwm(newValue);
    else if(parameterName == "PWM1")
        vco2.setPwm(newValue);
}

void LatticeSynthProcessor::prepareProcessor(int /* sr */, int /* block */)
{

}

void LatticeSynthProcessor::startNote(int midiNoteNumber, float velocity)
{
    setMidiNoteNumber(midiNoteNumber);
    if(velocity != 0 && isNoteOn == false)
    {
        isNoteOn = true;
    }
}

void LatticeSynthProcessor::stopNote (float velocity)
{
    if(velocity == 0)
    {
        isNoteOn = false;
    }
}

void LatticeSynthProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void LatticeSynthProcessor::process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues)
{
    const float freqVco1 = static_cast<float>(getMidiNoteInHertz(getMidiNoteNumber(), 440));
    const float freqVco2 = static_cast<float>(getMidiNoteInHertz(getMidiNoteNumber()+vco1.getSemitones(), 440));
    vco1.setBlockSize(blockSize);
    vco2.setBlockSize(blockSize);
    const std::vector<float> &out1 = vco1(1, freqVco1*vco1.getDetune(), isNoteOn);
    const std::vector<float> &out2 = vco2(1, freqVco2, isNoteOn);
    for(int i = 0; i < blockSize ; i++)
      for(int chan = 0 ;  chan < numChannels; chan++)
          buffer[chan][i] = out1[i] * mix + (out2[i] * (1.f - mix));
}



// the class factories
extern "C" LatticeProcessorModule* create(){     return new LatticeSynthProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }

