
#include "SpecSampProcessor.h"
#include <iterator>

static const int dm = 8;
std::atomic<bool> SpecSampProcessor::loading = false;
std::atomic<bool> SpecSampProcessor::ready = true;
std::vector<std::vector<Aurora::specdata<float>>>
SpecSampProcessor::samp(1, std::vector<Aurora::specdata<float>>(Aurora::def_fftsize));

//======================================================================================
SpecSampProcessor::SpecSampProcessor() :
win(Aurora::def_fftsize), anal(win,win.size()/dm), syn(win,win.size()/dm),
shift(Aurora::def_sr,win.size()), del(win.size()/2 + 1),  out(win.size()/2 + 1),
att(0.1f), dec(0.1f),
sus(1.f), rel(0.1f), env(att,dec,sus,rel), hcnt(anal.hsize()), ta(win.size()/(dm*Aurora::def_sr))
{
    std::size_t n = 0;
    for(auto &s : win)
        s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}

LatticeProcessorModule::ChannelData SpecSampProcessor::createChannels()
{
    addChannel({"input", ChannelType::input });
    addChannel({"ouput", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData SpecSampProcessor::createParameters()
{
    addParameter({ "Base Note", {0, 127, 60, 1, 1}});
    addParameter({ "Fine Tune", {0.9439,1.0594, 1, 0.0001, 1}});
    addParameter({ "Start Pos", {0, 1, 0, 0.001, 1}});
    addParameter({ "Loop Start", {0, 1, 0, 0.001, 1}});
    addParameter({ "Loop End", {0,1, 1, 0.001, 1}});
    addParameter({ "Timescale", {0, 2, 1, 0.001, 1}});
    addParameter({ "Amp Smear", {0, 1., 0, 0.001, 1}});
    addParameter({ "Freq Smear", {0, 1., 0, 0.001, 1}});
    addParameter({ "Granulation", {1, 100, 1, 0.1, 1}});
    addParameter({ "Attack", {0, 1., 0.01, 0.001, 1}});
    addParameter({ "Decay", {0, 1., 0.01, 0.001, 1}});
    addParameter({ "Sustain", {0, 1., 1., 0.001, 1}});
    addParameter({ "Release", {0, 1., 0.1, 0.001, 1}});
    addParameter({ "Keep Formants", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
    addParameter({ "Load Sample", {0, 1, 0, 1, 1}, Parameter::Type::FileButton});
    return ParameterData(getParameters(), getNumberOfParameters());
}

void SpecSampProcessor::hostParameterChanged(const char* parameterID,
                                             const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Load Sample")
    {
        if(ready)
        {
            ready = false;
            std::cout << "File to load" << newValue << std::endl;
            auto samples = getSamplesFromFile(newValue);
            //std::cout << samples.numSamples << std::endl;
            if(samples.numSamples > 0) {
                loading = true;
                samp.resize(samples.numSamples/anal.hsize());
                std::cout << "frames: " << samp.size() << " : " << samples.numSamples
                << " : " << anal.hsize() << std::endl;
                std::size_t n = 0;
                std::vector<float> lfr(anal.hsize());
                for(auto &frame : samp) {
                    std::copy(samples.data[0] + n,
                              samples.data[0] + n + anal.hsize(),
                              lfr.begin());
                    frame = anal(lfr);
                    n += anal.hsize();
                }
                std::cout << "samples: " << n << "\n";
                if(n < samples.numSamples) {
                    std::fill(lfr.begin(), lfr.end(), 0);
                    std::copy(samples.data[0] + n,
                              samples.data[0] + samples.numSamples,
                              lfr.begin());
                    samp.push_back(anal(lfr));
                }
            }
            loading = false;
        }
        okToDraw = true;
    }
}

void SpecSampProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    //
    if(paramName == "Amp Smear") {
        float par = getParameter(paramName);
        cfa = par > 0 ? std::pow(0.5, ta/par) : 0 ;
        //std::cout << ta << std::endl;
    } else if(paramName == "Freq Smear") {
        float par = getParameter(paramName);
        cff = par  > 0 ? std::pow(0.5, ta/par) : 0 ;
    }
    updateParameter(paramName, newValue);
}

void SpecSampProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
    anal.reset(sr);
    syn.reset(sr);
    hcnt = anal.hsize();
    shift.reset(sr);
    fs = sr;
    ta = win.size()/(dm*fs);
}

void SpecSampProcessor::startNote(int midiNoteNumber, float velocity )
{
    setMidiNoteNumber(midiNoteNumber);
    const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
    att = getParameter("Attack");
    dec = getParameter("Decay");
    rp = (getParameter("Start Pos") < getParameter("Loop End") ?
          getParameter("Start Pos") : getParameter("Loop End"))*samp.size();
    note_on = true;
}

void SpecSampProcessor::stopNote (float /* velocity */)
{
    note_on = false;
    ready = true;
    env.release(getParameter("Release"));
}

void SpecSampProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void SpecSampProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
    if(samp.size() > 1) {
        syn.vsize(blockSize);
        env.vsize(blockSize);
        sus = getParameter("Sustain");
        auto &e = env(note_on);
        if(hcnt >= anal.hsize()) {
            for(auto &bin : out) bin.amp(0);
        }
        
        const float decim = getParameter("Granulation");
        const float hops = anal.hsize()*decim;
        
        if(hcnt >= hops && !loading) {
            const float afac = decim < 4 ? decim : 4;
            const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
            const float base = getMidiNoteInHertz(getParameter("Base Note"), 440);
            shift.lock_formants(getParameter("Keep Formants"));
            shift(samp[(int)rp],freq*getParameter("Fine Tune")/base);
            float beg = getParameter("Loop Start")*samp.size();
            float end = getParameter("Loop End")*samp.size();
            if(end <= beg) beg = end;
            rp += getParameter("Timescale")*decim;
            rp = rp < end ? rp : beg;
            hcnt -= hops;
            std::size_t n = 0;
            for(auto &bin : shift.frame()) {
                del[n].freq(bin.freq()*(1 - cff) + del[n].freq()*cff);
                out[n].freq(del[n].freq());
                del[n].amp(bin.amp()*(1 - cfa) + del[n].amp()*cfa);
                out[n].amp(del[n].amp()*e[0]*afac);
                n++;
            }
        }
        auto &s = syn(out);
        std::copy(s.begin(),s.end(), buffer[0]);
        hcnt += blockSize;
    }
}


const char* SpecSampProcessor::getSVGXml()
{
    const float width = 256;
    const float height = 96;
    svg::Dimensions dimensions(width, height);
    svg::Document doc("specsamp.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
    svg::Polyline svgPath(svg::Fill(), svg::Stroke(2, svg::Color("#00ABD1"), 1));
    
    
    std::vector<float> amps(win.size()/2 + 1);
    for(auto &frame : samp) {
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
    //std::cout << scal << std::endl;
    for(auto &amp : amps) {
        amp *= scal;
	amp = 20*std::log10(amp);
        svgPath << svg::Point(n++,-(amp+96));
        if(n == width) break;
    }
    
    doc << svgPath;
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
