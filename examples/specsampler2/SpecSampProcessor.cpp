
#include "SpecSampProcessor.h"
#include <iterator>

static const int dm = 8;

//======================================================================================
SpecSampProcessor::SpecSampProcessor() :
win(Aurora::def_fftsize), samp(4, Aurora::SpecTable<float>(win,win.size()/dm)), syn(win,win.size()/dm),
players(2), del(win.size()/2 + 1),
out(win.size()/2 + 1), g({1.f,1.f}),
att(0.1f), dec(0.1f), sus(1.f), rel(0.1f), env(att,dec,sus,rel), hcnt(samp[0].hsize()),
ta(win.size()/(dm*Aurora::def_sr)), sparams(2)
{
    std::size_t n = 0;
    for(auto &s : win)
        s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
    
    
}

LatticeProcessorModule::ChannelData SpecSampProcessor::createChannels()
{
    addChannel({"output", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData SpecSampProcessor::createParameters()
{
    addParameter({ "Amp Morph", {0, 1., 0, 0.001, 1}});
    addParameter({ "Freq Morph", {0, 1., 0, 0.001, 1}});
    addParameter({ "Amp Smear", {0, 1., 0, 0.001, 1}});
    addParameter({ "Freq Smear", {0, 1., 0, 0.001, 1}});
    addParameter({ "Granulation", {1, 100, 1, 0.1, 1}});
    addParameter({ "Attack", {0, 1., 0.01, 0.001, 1}});
    addParameter({ "Decay", {0, 1., 0.01, 0.001, 1}});
    addParameter({ "Sustain", {0, 1., 1., 0.001, 1}});
    addParameter({ "Release", {0, 1., 0.1, 0.001, 1}});
    addParameter({ "Reset", {0, 1, 0, 1, 1}, Parameter::Type::Momentary});
    for(auto &p : sparams.pnames) {
        addParameter({ p[0].c_str(), {0, 127, 60, 1, 1}});
        addParameter({ p[1].c_str(), {0.9439,1.0594, 1, 0.0001, 1}});
        addParameter({ p[2].c_str(),  {-1000, 1000, 0, 1, 1}});
        addParameter({ p[3].c_str(), {0.01, 2, 1, 0.001, 1}});
        addParameter({ p[4].c_str(), {0, 1, 1, 0.001, 1}});
        addParameter({ p[5].c_str(), {0, 1, 0, 0.001, 1}});
        addParameter({ p[6].c_str(), {0, 1, 0, 0.001, 1}});
        addParameter({ p[7].c_str(), {0,1, 1, 0.001, 1}});
        addParameter({ p[8].c_str(), {-2, 2, 1, 0.001, 1}});
        addParameter({ p[9].c_str(), {0, 1, 0, 1, 1},
	      LatticeProcessorModule::Parameter::Type::Switch});
        addParameter({ p[10].c_str(), {0, 1, 0, 1, 1}, Parameter::Type::FileButton});
        addParameter({ p[11].c_str(), {0, 1, 0, 1, 1}, Parameter::Type::Momentary});
        
    }
    
    
    return ParameterData(getParameters(), getNumberOfParameters());
}

void SpecSampProcessor::loadSpec(Aurora::SpecTable<float> &samp,
				 const char* newValue) {
  loading = true;
  if(!getVoiceNum())
    {     
      std::cout << getVoiceNum()  << std::endl;
      std::cout << "File to load " << newValue << std::endl;
      auto samples = getSamplesFromFile(newValue);
      if(samples.numSamples > 0) {
	std::vector<float> in(samples.numSamples);
	std::copy(samples.data[0],
		  samples.data[0]+samples.numSamples,
		  in.begin());
        auto nframes = samp(in);
	std::cout << "Frames loaded: " << nframes << std::endl;
      }
    }
  loading = false;
  okToDraw = true;
}


void SpecSampProcessor::hostParameterChanged(const char* parameterID,
                                             const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Load Sample 1") loadSpec(samp[0], newValue);
    else if(paramName == "Load Sample 2") loadSpec(samp[1], newValue);
}

void SpecSampProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Amp Smear") {
        float par = getParameter(paramName);
        cfa = par > 0 ? std::pow(0.5, ta/par) : 0 ;
    } else if(paramName == "Freq Smear") {
        float par = getParameter(paramName);
        cff = par  > 0 ? std::pow(0.5, ta/par) : 0 ;
    } else if(paramName == "Reset") {
        float par = getParameter(paramName);
        if(par) doReset = true;
    }
else {
    std::size_t n = 0;
    for(auto &p : sparams.pnames) {
      float par = getParameter(paramName);
      if(paramName == p[0]) {
	players[n].basefreq(getMidiNoteInHertz(par, 440));
      }
      else if(paramName == p[1])
	players[n].finetune(par);
      else if(paramName == p[2])
	players[n].freqshift(par);
      else if(paramName == p[3])
	players[n].formscal(par);
      else if(paramName == p[4])
	g[n] = par;
      else if(paramName == p[5])
	players[n].start(par);
      else if(paramName == p[6])
	players[n].loopbeg(par);
      else if(paramName == p[7])
	players[n].loopend(par);
      else if(paramName == p[8])
	players[n].timescale(par);
      else if(paramName == p[9])
	players[n].keepform(par);
      else if(paramName == p[11]) {
        if(getParameter(paramName)) {
	  if(!getVoiceNum()) {
	    updateHostParameter(p[10].c_str(), "");
	    samp[n].clear();
	    okToDraw = true;
	  }	    
	}
      }
      n++;
    }
  }
}

void SpecSampProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
  syn.reset(sr);
  hcnt = samp[0].hsize();
  for(auto &player: players)
      player.reset(sr);
  fs = sr;
  ta = win.size()/(dm*fs);
  for(auto &smp: samp)
      smp.sr(sr);
}

void SpecSampProcessor::startNote(int midiNoteNumber, float velocity )
{
  setMidiNoteNumber(midiNoteNumber);
  att = getParameter("Attack");
  dec = getParameter("Decay");
  std::size_t n = 0;
  for(auto &player : players) 
    player.onset(getSamp(n++).size());
  note_on = true;
}

void SpecSampProcessor::stopNote (int, float /* velocity */)
{
    note_on = false;
    env.release(getParameter("Release"));
}

void SpecSampProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void SpecSampProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
    
     if(doReset) {
     for(auto &player : players) player.reset(fs);
     syn.reset(fs);
     doReset = false;
     }
     
     int smps = blockSize, hsize = samp[0].hsize(), offs = 0;
     if(smps > hsize) blockSize = hsize;
     
     while(smps > 0) {
     
     syn.vsize(blockSize);
     env.vsize(blockSize);
     sus = getParameter("Sustain");
     auto &e = env(note_on);
     if(hcnt >= samp[0].hsize()) {
     for(auto &bin : out) bin.amp(0);
     }
     
     const float decim = getParameter("Granulation");
     const float hops = samp[0].hsize()*decim;
     
     if(hcnt >= hops && !loading) {
     std::size_t n = 0;
     const float afac = decim < 4 ? decim : 4;
     const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
     auto &frame0 = players[0](getSamp(0),freq);
     auto &frame1 = players[1](getSamp(1),freq);
     const float xa = getParameter("Amp Morph");
     const float xf = getParameter("Freq Morph");
     
     hcnt -= hops;
     for(auto &bin : out) {
     bin.freq(frame0[n].freq()*(1-xf) + frame1[n].freq()*xf);
     del[n].freq(bin.freq()*(1 - cff) + del[n].freq()*cff);
     bin.freq(del[n].freq());
     bin.amp(frame0[n].amp()*(1-xa)*g[0] + frame1[n].amp()*xa*g[1]);
     del[n].amp(bin.amp()*(1 - cfa) + del[n].amp()*cfa);
     bin.amp(del[n].amp()*e[0]*afac);
     n++;
     }
     }
     auto &s = syn(out);
     hcnt += blockSize;
     std::copy(s.begin(),s.end(), buffer[0]+offs);
     offs += blockSize;
     smps -= hsize;
     blockSize = smps < hsize ? smps : hsize;
     }
}


const char* SpecSampProcessor::getSVGXml()
{
    const float width = 256;
    const float height = 96;
    svg::Dimensions dimensions(width, height*2);
    svg::Document doc("specsamp.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
    svg::Polyline svgPath1(svg::Fill(), svg::Stroke(2, svg::Color("#00ABD1"), 1));
    svg::Polyline svgPath2(svg::Fill(), svg::Stroke(2, svg::Color("#ABD100"), 1));
    
    std::vector<float> amps(win.size()/2 + 1);
    
    if(getSamp(0).size()) {
      for(auto &frame : getSamp(0)()) {
            std::size_t n = 0;
            for(auto &bin : frame) {
                amps[n++] += bin.amp();
            }
        }
        std::size_t n = 0;
        float max = 0;
        for(auto &amp : amps)
            if(amp > max) max = amp;
        float scal = 1./max;
        for(auto &amp : amps) {
            amp *= scal;
            amp = 20*std::log10(amp);
            svgPath1 << svg::Point(n++,-(amp+96)/2);
            if(n == width) break;
        }
        doc << svgPath1;
    }
    
    if(getSamp(1).size()) {
        std::fill(amps.begin(), amps.end(), 0);
        for(auto &frame : getSamp(1)()) {
            std::size_t n = 0;
            for(auto &bin : frame) {
                amps[n++] += bin.amp();
            }
        }
        std::size_t n = 0;
        float max = 0;
        for(auto &amp : amps)
            if(amp > max) max = amp;
        float scal = 1./max;
        for(auto &amp : amps) {
            amp *= scal;
            amp = 20*std::log10(amp);
            svgPath2 << svg::Point(n++,-(amp+96)/2);
            if(n == width) break;
        }
        doc << svgPath2;
    }
    
    svgText = doc.toString();
    return svgText.c_str();
    
}



// the class factories
#ifdef WIN32
extern "C" 
{
__declspec(dllexport) LatticeProcessorModule* create() { return new SpecSampProcessor; }
};

extern "C" 
{
__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new SpecSampProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
