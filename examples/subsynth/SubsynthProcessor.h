#pragma once
#include "LatticeProcessorModule.h"
#include "../../aurora/include/BlOsc.h"
#include "../../aurora/include/Env.h"

#include <iterator>
#include <array>

namespace Aurora {
const TableSet<float> sawtooth(SAW);
const TableSet<float> square(SQUARE);
const Aurora::TableSet<float> triangle(TRIANGLE);
const TableSet<float> *sawtab = &sawtooth;
const TableSet<float> *sqrtab = &square;
const TableSet<float> *tritab = &triangle;

 struct Oscil { 
   Aurora::BlOsc<float> osc;
   Aurora::BlOsc<float> oscb;
   std::vector<float> sig;
   float freq, fine, fm, pwm, amp;
   int wave;
   ParamSmooth smooth;

   float midi2freq(float nn) {
    return 440*std::pow(2.f, (nn - 69.f)/12.f);
   }


 Oscil() : osc(sawtab), oscb(sawtab), sig(def_vsize), freq(60.),
     fine(0.), fm(0.), pwm(0.5), amp(0), wave(0) { };

   void set_wave(int wav) {
     wave = wav;
     switch(wave) {
       case 1:
	 osc.waveset(sqrtab);
	 break;
       case 2:
	 osc.waveset(tritab);
	 break;
       default:	 
	 osc.waveset(sawtab);
	 break;	 
     }
   }
   
     const std::vector<float>
       &operator()(float amp, const std::vector<float> &fm) {
       sig.resize(fm.size());
       if(wave < 3) {
	 auto &a = osc(amp, fm);
	 oscb(amp,fm);
	 std::copy(a.begin(), a.end(), sig.begin());   
	 return sig;
       }
       else {
	float pwms = smooth(pwm,0.01f, osc.fs()/fm.size());
        float off = amp*(2*pwms - 1.f);
	auto &a = osc(amp,fm,pwms);
	auto &b = oscb(amp,fm);
	std::size_t j = 0;
        for(auto &s :sig) {
	  s = a[j] - b[j] + off;
          j++;
	}
	return sig;		
       }
     } 
 };
 }

struct OscParam {
  std::array<const char *, 6> params;
  std::vector<std::vector<std::string>> pnames;

  OscParam(std::size_t np) : params({ "Coarse Freq ", "Fine Tune ", "FM Amount ", "PWM ", "Waveform ", "Osc "}),
    pnames(np) {
    std::size_t n = 0;
    char mem[4];
    for(auto &names : pnames) {
    std::size_t j = 0;
    names.resize(params.size());
    for(auto &name : names) {
       sprintf(mem,"%lu", n+1);
       std::string cc = mem;
      name = params[j++] + cc;
    }
    n++;
    }
  }
};
 

class SubsynthProcessor : public LatticeProcessorModule
{
    OscParam oparams;
    std::vector<Aurora::Oscil> oscs;
    std::vector<float> buf;
    float att, dec, sus, rel;
    Aurora::Env<float> aenv;

 
public:
    SubsynthProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    /* Called by the host when a note is started */
    void startNote(int midiNoteNumber, float velocity) override;
    
    /* Called by the host when a note is stoped */
    void stopNote (float velocity) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void processSynthVoice(float** buffer, int numChannels, std::size_t blockSize) override;

    const char* getModuleName() override
    {
        return "Subtractive Synth";
    }

    /* Is a synth */
    bool isSynth() override
    {
        return true;
    }

    float getTailOffTime() override
    {
      return getParameter("Release");
    }
    
    int getModuleType() override
    {
        return ModuleType::SynthProcessor::uncategorised;
    }
    
private:
    bool isNoteOn = false;
};

