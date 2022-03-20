
#include "BlSynthProcessor.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"


BlSynthProcessor::Synth::Synth(float rt, float sr)
: 
squareWave(Aurora::SQUARE),
triangleWave(Aurora::TRIANGLE),
sawWave(Aurora::SAW),
sineWave(Aurora::def_ftlen),
env(Aurora::ads_gen(att, dec, sus), rt, Aurora::def_sr),
sinOsc(&sineWave,sr),
osc(&sawWave, sr),
sawOsc1(&sawWave, sr),
sawOsc2(&sawWave, sr),
mix(),
pwmTone(sr),
pwmChanges(1)
{
    pwmChanges[0] = 0.5;
    std::size_t n = 0;
    for (auto &s : sineWave)
    {
      s = static_cast<float>(std::sin((Aurora::twopi / sineWave.size()) * n++));
    }
    
    
}

const std::vector<float>& BlSynthProcessor::Synth::operator()(float a, float f, bool gate)
{
    if(currentWave == 0)
    {
        return env(sinOsc(a, f), gate);
    }
    else if(currentWave == 2)
    {
        auto &pwmSmooth = pwmTone(pwmChanges, 10.f);
        float off = a*(2*pwmSmooth[0] - 1.f);
        auto &m = mix(mix(sawOsc1(a, f, pwmSmooth[0]), sawOsc2(-a, f)), off);
        return env(m, gate);
    }
    else
    {
        return env(osc(a, f), gate);
    }
}

void BlSynthProcessor::Synth::setWaveform(int waveForm)
{
    currentWave = waveForm;
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

void BlSynthProcessor::Synth::setBlockSize(std::size_t blockSize)
{
    pwmTone.vsize(blockSize);
    sawOsc1.vsize(blockSize);
    sawOsc2.vsize(blockSize);
    sinOsc.vsize(blockSize);
    osc.vsize(blockSize);
    env.vsize(blockSize);
}

void BlSynthProcessor::Synth::setSampleRate(std::size_t blockSize)
{
    pwmTone.reset(blockSize);
    sawOsc1.reset(blockSize);
    sawOsc2.reset(blockSize);
    sinOsc.reset(blockSize);
    osc.reset(blockSize);
    env.reset(blockSize);
}

//======================================================================================
BlSynthProcessor::BlSynthProcessor()
:synth(.1f, 44100)
{
    
}


void BlSynthProcessor::createChannelLayout(DynamicArray<const char*> &inputs, DynamicArray<const char*> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}

void BlSynthProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({"Attack", {0, 1, 0.4, 0.001, 1}});
    parameters.push_back({"Decay", {0, 2, 0.1, 0.001, 1}});
    parameters.push_back({"Sustain", {0, 1, 0.8, 0.001, 1}});
    parameters.push_back({"Release", {0, 3, 0.1, 0.001, 1}});
    parameters.push_back({"Wave", {0, 3, 1, 1, 1}});
    parameters.push_back({"PWM", {0.01, .999, .5, 0.001, 1}});
    parameters.push_back({"Detune", {.5, 2, 1, 0.001, 1}});
}

void BlSynthProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
    auto parameterName = getParameterNameFromId(parameterID);
    
    if(parameterName == "Wave")
    {
        synth.setWaveform(int(newValue));
		waveform = int(newValue);
		okToDraw = true;
    }
    else if(parameterName == "Attack")
    {
        synth.setAttack(newValue);
    }
    else if(parameterName == "Decay")
    {
        synth.setDecay(newValue);
    }
    else if(parameterName == "Sustain")
    {
        synth.setSustain(newValue);
    }
    else if(parameterName == "Release")
    {
        synth.setRelease(newValue);
    }
    else if(parameterName == "Detune")
    {
        synth.setDetune(newValue);
    }
    else if(parameterName == "PWM")
    {
        synth.setPwm(newValue);
    }
    
}

void BlSynthProcessor::prepareProcessor(int sr, std::size_t block)
{
	synth.setSampleRate(sr);
	synth.setBlockSize(block);
}

void BlSynthProcessor::startNote(int noteNumber, float velocity)
{
    setMidiNoteNumber(noteNumber);
    if(velocity != 0 && isNoteOn == false)
    {
        isNoteOn = true;
    }
}

void BlSynthProcessor::stopNote (float /*velocity*/)
{
    isNoteOn = false;
}

void BlSynthProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void BlSynthProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
    const float freq = static_cast<float>(getMidiNoteInHertz(getMidiNoteNumber(), 440));
    synth.setBlockSize(blockSize);

    auto &out = synth(1, freq*synth.getDetune(), isNoteOn);
        
    for(int i = 0; i < blockSize ; i++)
      for(int chan = 0 ;  chan < numChannels; chan++)
          buffer[chan][i] = out[i];
}

std::string BlSynthProcessor::getSVGXml()
{
	const float width = 200;
	const float height = 40;
	svg::Dimensions dimensions(width, height);
	svg::Document doc("bl.svg", svg::Layout(dimensions, svg::Layout::TopLeft));

	svg::Polyline svgPath(svg::Fill(), svg::Stroke(2, svg::Color("#00ABD1"), 1));

	if (waveform == 0)
	{
		auto cycles = synth.sineWave.size() * 5;
		for (int i = 0; i < cycles; i += (synth.sineWave.size() / 10))
		{
			double x = remap(i, 0.f, static_cast<float>(cycles), 0.f, width);
			double y = remap(synth.sineWave[i % synth.sineWave.size()] * .6f, -1, 1, 0, height);
			auto pos = svg::Point(x, y);
			svgPath << pos;
		}
	}
	else
	{

		std::vector<float> samples;

		if (waveform == 1)
		{
			samples = synth.sawWave.func(10);
		}
		else if (waveform == 2)
		{
			samples = synth.squareWave.func(10);
		}
		else if (waveform == 3)
		{
			samples = synth.triangleWave.func(10);
		}

		auto cycles = samples.size() * 5;
		for (int i = 0; i < cycles; i += samples.size() / 10)
		{
			double x = remap(i, 0.f, static_cast<float>(cycles), 0.f, width);
			double y = remap(samples[i % samples.size()] * .6f, -1, 1, 0, height);
			auto pos = svg::Point(x, y);
			svgPath << pos;
		}
	}

	doc << svgPath;
	return doc.toString();
}
// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new BlSynthProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new BlSynthProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
