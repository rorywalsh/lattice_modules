#pragma once
#include "LatticeProcessorModule.h"
#include "BlOsc.h"
#include "Env.h"
#include "FourPole.h"
#include "TwoPole.h"


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
   float freq, fine, fm, pwm, amp, lfo1, lfo2, env;
   int wave;
   ParamSmooth smooth;
   ParamSmooth ampsm;

   float midi2freq(float nn) {
    return 440*std::pow(2.f, (nn - 69.f)/12.f);
   }


 Oscil() : osc(sawtab), oscb(sawtab), sig(def_vsize), freq(60.),
     fine(0.), fm(0.), pwm(0.5), amp(0.5), wave(0), lfo1(0), lfo2(0), env(0) { };

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
       float a = ampsm(amp,0.01f,osc.fs()/fm.size());
       if(wave < 3) {
	 auto &a = osc(amp, fm);
	 oscb(a,fm);
	 std::copy(a.begin(), a.end(), sig.begin());   
	 return sig;
       }
       else {
	float pwms = smooth(pwm,0.01f, osc.fs()/fm.size());
        float off = a*(2*pwms - 1.f);
	auto &s1 = osc(a,fm,pwms);
	auto &s2 = oscb(a,fm);
	std::size_t j = 0;
        for(auto &s :sig) {
	  s = s1[j] - s2[j] + off;
          j++;
	}
	return sig;		
       }
     } 
 };

 const struct Tables {
   std::vector<float> utri;
   std::vector<float> btri;
   std::vector<float> saw;
   std::vector<float> sqr;
   std::vector<float> sine;
   std::vector<float> *tabs[5];

 Tables() :
   utri(8192), btri(8192), saw(8192), sqr(8192), sine(8192),
     tabs{&sine,&utri,&btri,&saw,&sqr}
       {
     std::size_t n = 0;
     for(auto &wv : sine) {
       wv = std::sin(twopi*n/sine.size());
       utri[n] = n < utri.size()/2 ? (2.f*n)/utri.size() : 1 - (2.*n - utri.size())/utri.size();
       btri[n] = utri[n]*2 - 1.;
       saw[n] = float(n)/saw.size();
       sqr[n] = n < sqr.size()/2 ? 1. : -1;
       n++;
     }
   }
  } lfotables;

 struct LFO {
   Osc<float,lookup> osc;
   LFO() : osc(&lfotables.sine, def_sr) { };

   auto &operator()(float freq)  {
     return osc(1, freq);
   }

   void set_wave(int w) {
     osc.table(lfotables.tabs[w]);
   }   
 };
 
 }

struct OscParam {
  std::array<const char *, 9> params;
  std::vector<std::vector<std::string>> pnames;

OscParam(std::size_t np) : params({ "Coarse Freq ", "Fine Tune ", "FM Amount ",
      "LFO1 Amount ", "LFO2 Amount ", "Aux Env ",  "PW ", "Waveform ", "Osc "}),
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
    std::vector<float> buf, cf1, cf2;
    float att, dec, sus, rel;
    Aurora::Env<float> aenv;
    float xatt, xdec, xsus, xrel;
    Aurora::Env<float> xenv;
    Aurora::FourPole<float> lp;
    Aurora::TwoPole<float> svf;
    Aurora::LFO lfo1, lfo2;
    float mvel;
    float pbend = 0;
    ParamSmooth fsmooth;
    ParamSmooth bsmooth;
 
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
        return "Subtractive Synth II";
    }

    /* Is a synth */
    bool isSynth() override
    {
        return true;
    }

    float getTailOffTime() override
    {
      return getParameter("Amp Release");
    }
    
    int getModuleType() override
    {
        return ModuleType::SynthProcessor::uncategorised;
    }

     void pitchBendChange(int newValue) override
    {
      pbend = (newValue - 64)/128.;
    }
    
private:
    bool isNoteOn = false;
};

