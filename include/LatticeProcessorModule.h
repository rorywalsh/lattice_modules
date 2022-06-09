#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <atomic>
#include <functional>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//silence unused wanrings..
template <typename... Types>
void unused(Types&&...) noexcept {}

class ParamSmooth {
  float y;
  float t;
  float cf;
  float sr;

 public:
 ParamSmooth() : y(0), t(0), cf(0), sr(0) {};

  /** smoothing function \m
   *   x - input \n
   *   ti - smooth time in secs \n
   *   rate - signal processing rate
   */
  float operator() (float x, float ti, float rate = 44100/64.f) {
    if(ti != t || rate != sr) {
      t = ti;
      sr = rate;
      if(t > 0 && sr > 0)
       cf = std::pow(0.5, 1./(t*sr));
      else cf = 0;
    }
    return (y = x*(1-cf) + y*cf);
  }
};

/** @class LatticeProcessorModule
 * @brief Base class for all modules
 *
 * This is the main base class for all Lattice modules. Every module must inherit from this class
 *
 */
class LatticeProcessorModule
{

public:
    /** @struct ModuleParameter
    * @brief Module parameters
    *
    * This is the main Lattice parameter struct. As as many of these to as you need to the `std::vector<ModuleParameter>& parameters` that is passed to the `LatticeModuleProcessor::createParameters(std::vector<ModuleParameter>& parameters)` method. Each parameter added here will be accessible in the Lattice graph when you double click the processor module.
    *
    */
    struct Parameter
    {
        /*! Parameter Type enum */
        enum Type {
        Trigger = -99, /*!< A trigger button - shown only a single state button */
            Switch =  -98,  /*!< A switch button - shown as an dual state button */
            Slider = -97, /*!< A slider (default) - shown as a horizontal slider */
            FileButton = -96, /*!< A non-automatable button that launches a file browser dialogue */
            Momentary = -95, /*!< A momentary button - shown only a single state button - will return a value of 1 for as long as the button is being pressed */
            AutomationMode = -94
        };

        /** Name of your module parameter - this is the name that will appear in the module editor in Lattice */
        const char* parameterName;
        /** A vector holding min, max, defaultValue, increment, and skew values for your parameter. A skew of 1 is linear, while a skew of less than one will result in non-linear behaviour. */
        struct Range {
            float min = 0;
            float max = 1;
            float defaultValue = 0;
            float increment = 0.01f;
            float skew = 1;
            Range(float m, float n, float d, float i, float s)
                : min(m), max(n), defaultValue(d), increment(i), skew(s) {}
            Range() {
                min = 0;
                max = 1;
                defaultValue = .5f;
                skew = 1;
                increment = 0.001f;
            }
        };

        Range range;

        /*! Creates a module parameter. You can add as many of these as you wish to your module by pushing to the `std::vector<ModuleParameter>& parameters` argument that gets passed to LatticeProcessorModule::createParameters()

    ```
    void ChorusProcessor::createParameters(std::vector<ModuleParameter> &parameters)
    {
        parameters.push_back({ "Delay Time (L)", {0, 5, .017f, .001f, 1}});
        parameters.push_back({ "LFO Frequency (L)", {0, 10, .93, .001f, 1}});
        parameters.push_back({ "Delay Time (R)", {0, 5, .013f, .001f, 1}});
        parameters.push_back({ "LFO Frequency (R)", {0, 10, .083f, .001f, 1} });
    }
    ```
    */

        Parameter(const char* name, Range paramRange, Parameter::Type type = Type::Slider, bool visible = false)
            :parameterName(name), range(paramRange), paramType(type), showInputPin(visible)
        {
            
        }

        Parameter& withLabel(const char* l)
        {
            label = l;
            return *this;
        }

        Parameter& withHint(const char* h)
        {
            hint = h;
            return *this;
        }

        /** Set this to use a different label than the one given to parameterName */
        const char* label = "";
        /** Set this to add a postfix to the parameter name as it appears in the module editor window.*/
        const char* hint = "";
        /** Set the type of basic UI element to use for the parameter. If you select a a switch or trigger, many sure your parameter range is between 0 and 1, and the increment is set to 1*/
        Type paramType = Type::Slider;
        
        bool showInputPin = false;
    };


    /** @struct HostData
     * @brief Data from the host
     *
     * One of these is passed to the `LatticeProcessorModule::process()` method each time is it called. This can be used to query everything from the current host BPM to whether or not the host is actually playing. (Note: only available in effect plugins)
     */
    struct HostData
    {
        /** The current host BPM */
        double bpm = 120;
        /** The time signature numerator */
        int timeSigNumerator = 4;
        /** The time signature denomiator */
        int timeSigDenomiator = 4;
        /** The current performance time in samples */
        long int timeInSamples = 0;
        /** Indicates if the host is playing */
        bool isPlaying = false;
        /** Indicates if the host is recording */
        bool isRecording = false;
        /** Indicates if the host is looping */
        bool isLooping = false;
        /** The current performance time in seconds */
        double timeInSeconds = 0;
    };

    /** @struct LatticeMidiMessage
     * @brief MIDI message class
     *
     * A simple MIDI message struct. `LatticeMidiMessage` vectors of can be accessed in the `<LatticeProcessorModule::process()>`

     */

    struct LatticeMidiMessage {

        /*! Midi Message Type enum */
        enum Type {
            noteOn = 1,/** noteOn message */
            noteOff /** noteOff message */
        };

        /** Message type, on or off*/
        Type msgType = Type::noteOff;
        /** MIDI note number */
        int note = 60;
        /** MIDI channel */
        int channel = 0;
        /** MIDI velocity as a float between 0 and 1 */
        float velocity;

        /** Creates a simple MIDI message that can be added to a incoming MIDI vector in process()
         */
        LatticeMidiMessage(LatticeMidiMessage::Type type, int chan, int noteNum, float vel)
            : msgType(type), note(noteNum), channel(chan), velocity(vel)
        {}

    };

    /*! ModuleType enum */
    struct ModuleType
    {
        struct AudioProcessor
        {
            static constexpr int uncategorised = 0;
            static constexpr int dynamic = 2;
            static constexpr int filters = 3;
            static constexpr int spectral = 6;
            static constexpr int delay = 7;
            static constexpr int nonlinear = 8;
        };
        struct MidiProcessor
        {
            static constexpr int uncategorised = 100;
            static constexpr int generator = 101;
            static constexpr int modifier = 102;
            
        };
        struct SynthProcessor
        {
            static constexpr int uncategorised = 200;
            static constexpr int distortion = 201;
            static constexpr int spectral = 202;
            static constexpr int time = 203;
            static constexpr int nonlinear = 204;
            
        };

        struct Automator{
            static constexpr int uncategorised = 300;
        };
        
        struct SamplerProcessor
        {
            static constexpr int uncategorised = 400;
        };
        
//        synthProcessor = 0,/** a module that will form part of a polyphonic or monophonic synth. This type of module can not only generate audio, but can also process incoming. See LatticeProcessorModule::getNumberOfVoices() for details on how to limit the number of voices. */
//        audioProcessor,/** an audio processor. This can be used to process or generate audio */
//        midiProcessor, /** a Midi based processor - can parse and modify incoming Midi data or generate new Midi streams on the fly*/
//        automator /** an automation processor. */
    };

    enum ChannelType {
        input = 0,
        output
    };

    struct Channel {
        const char* name;
        ChannelType type;
        Channel(const Channel &c) : name(c.name), type (c.type) {}
        Channel(const char* n, ChannelType t) :name(n), type(t) {}
        Channel& operator=(Channel other)
        {
            std::swap(name, other.name);
            std::swap(type, other.type);
            return *this;
        }
    };

    struct ChannelData {
        Channel* data;
        std::size_t size;
        ChannelData(Channel* n, std::size_t  s) : data(n), size(s) { }
    };

    struct ParameterData {
        Parameter* data;
        std::size_t size;
        ParameterData(Parameter* n, std::size_t  s) : data(n), size(s) { }
    };

    LatticeProcessorModule() {}

    /**
     * This method is called by the host to create channel layout. Override to configure your plugin's inputs and outputs
    * @param[in] inputs Fill this with your input channels.
     * @param[in] outputs Fill this with your output channels.
    */
    virtual ChannelData createChannels()
    {
        channels.push_back({ "Input 1", ChannelType::input });
        channels.push_back({ "Input 2", ChannelType::input });
        channels.push_back({ "Output 2", ChannelType::output });
        channels.push_back({ "Output 2", ChannelType::output });
        return ChannelData(getChannels(), getNumberOfChannels());
    }

    /**
     * This method is called by the host to create the plugin parameters. Override and fill the `ModuleParameter` vector
     * @param[in] parameters Fill this with your modules parameters.
     */
    virtual ParameterData createParameters()
    {
        addParameter({ "Parameter 1", { 0, 1, 0.001f, 0.001f, 1 } });
        return ParameterData(getParameters(), getNumberOfParameters());
    }

    /**
     * This function is called by the host before playback/performance. Override it to set your plugin's sample rate and block size
     * @param[in] sr The sample rate as set by the host
     * @param[in] blockSize The blocks size, as set by the host. This is subject to change
     */
    virtual void prepareProcessor(int sr, std::size_t blockSize)
    {
        unused(sr, blockSize);
    }


    /*!
    Buffer is an array of audio channels. Each channel array will be blockSize samples longer. You can access and modify the sample data like this

    @code
    for ( int i = 0 ; i < blockSize ; i++)
    {
        for ( int chan = 0 ; chan < numChannels; chan++)
        {
            //buffer[chan][i] = buffer[chan][i] * getParameter("Gain");
        }
    }
    @endcode

    @param[in] buffer An array of audio channels.
    @param[in] numChannels  The number of channels in the buffer array
    @param[in] blockSize The number of samples in each channel
    @param[in] info A struct filled with host information
    */
    virtual void process(float** buffer, int numChannels, std::size_t blockSize, const HostData info)
    {
        unused(buffer, numChannels, blockSize, info);
    }

    /*!
     Use this for MIDI based effects. Each channel array will be blockSize samples longer. This process is passed a reference to a vector of LatticeMidiMessages. (See above for an example of how to access the channel data.)
    * @param[in] buffer An array of audio channels.
    * @param[in] numChannels  The number of channels in the buffer array
    * @param[in] blockSize The number of samples in each channel
    * @param[in] midiMessages A LatticeMidiMessage vector. Read from this to access incoming MIDI data, and write to it to output MIDI data.
    * @param[in] info A struct filled with host information
    */
    virtual void processMidi(float** buffer, int numChannels, std::size_t blockSize, const HostData info, std::vector<LatticeMidiMessage>& midiMessages)
    {
        unused(buffer, numChannels, blockSize, midiMessages, info);
    }
    /*!
    This processing method is called by synth modules. No MIDI messages are passed directly to this function. Instead, the start and stop notes methods will be called. Lattice will allocating an instance of the module to each voice. Lattice will set the number of voices when it calls  getNumberOfVoices(). Buffer is an array of audio channels. Each channel array will be blockSize samples longer. This process is passed a reference to a vector of LatticeMidiMessages. (See above for an example of how to access the channel data.)
   * @param[in] buffer An array of audio channels.
   * @param[in] numChannels  The number of channels in the buffer array
   * @param[in] blockSize The number of samples in each channel
   */
    virtual void processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
    {
        unused(buffer, numChannels, blockSize);
    }

    virtual void processSamplerVoice(float** buffer, int numChannels, std::size_t blockSize)
    {
        unused(buffer, numChannels, blockSize);
    }

    
    /** Called by Lattice to set the number of voices
     * @return The maximum number of voices that can be played simultaneously.
    */
    virtual int getNumberOfVoices()
    {
        return 32;
    }

    /** Called by Lattice to determine if blocks sizes in synths should always match those of
    * the module's vectors size. It defaults to false, but can be important for synths that take
    * audio input.
    */
    virtual bool restrictBlockSize()
    {
        return false;
    }
    
    /** Called by the host when a float paremeter changes. The parameterID in this instance is a combination of the unique name for the module, assigned by the host, and the parameter name itself, i.e, 'Super Synth 11 - Attack'. Use the getParameterName() method to extract the parameter name - note that in the case of audio FX, you can just called getParameter() from your process block.
    * @param[in] parameterID The name of the parameter that has been update. This will also contain information about the module instance, so use getParameterName() to extract the actual module parameter name.
     * @param[in] newValue  The parameter value sent by the host.
    */
    virtual void hostParameterChanged(const char* parameterId, float newValue)
    {
        updateParameter(getParameterNameFromId(parameterId), newValue);
    }

    /** Called by the host when a string paremeter changes. The parameterID in this instance is a combination of the unique name for the module, assigned by the host, and the parameter name itself, i.e, 'Super Synth 11 - Attack'. Use the getParameterName() method to extract the parameter name - note that in the case of audio FX, you can just called getParameter() from your process block.
    * @param[in] parameterID The name of the parameter that has been update. This will also contain information about the module instance, so use getParameterName() to extract the actual module parameter name.
     * @param[in] newValue  The parameter value sent by the host.
    */
    virtual void hostParameterChanged(const char* parameterID, const char* newValue)
    {
        unused(parameterID, newValue);
    }

    /** called by the host when a note is started
     * @param[in] noteNumber Midi note number sent from host
     * @param[in] velocity Midi velocity in the range of 0 to 1
    */
    virtual void startNote(int noteNumber, float velocity)
    {
        unused(noteNumber, velocity);
    }

    /** called by the host when a note is started
     @return The time in second that any note wil last for. This is independent of your processor's release time, so make sure that you give enough time for your processor to release.
    */
    virtual float getTailOffTime()
    {
        return 2;
    }
    /** called by the host when a note is stoped
     * @param velocity Midi velocity in the range of 0 to 1
    */
    virtual void stopNote(float velocity)
    {
        unused(velocity);
    }

    /** called by the host when a the pitchbend wheel is moved
     * @param newValue integer in the range of 0 to 128
    */
    virtual void pitchBendChange(int newValue)
    {
        unused(newValue);
    }

    /** called by the host when a the pitchbend wheel is moved
     * @param controllerNumber the controller number
     * @param newValue integer in the range of 0 to 128
    */
    virtual void controllerChange(int controllerNumber, int newValue)
    {
        unused(controllerNumber, newValue);
    }

    /** Override this method to provide a unique name for the module
     * @return The name of the module as shown in Lattice
    */
    virtual const char* getModuleName()
    {
        return "ModuleName";
    }

    double getMidiNoteInHertz(const int noteNumber, const double aTuning = 440)
    {
        return aTuning * std::pow(2.0, static_cast<double>(noteNumber - 69) / 12.0);
    }

    ///@private
    /** called by the host to set the current MIDI note number */
    void setMidiNoteNumber(int noteNumber)
    {
        midiNoteNumber = noteNumber;
    }

    /** retrieves the current MIDI note number
     * @return the current MIDI note number
    */
    int getMidiNoteNumber()
    {
        return midiNoteNumber;
    }

    /** Call by Lattice to determine if the module is a synth of an effects (audio/MIDI) processor.
    * Set to true if module is a synth.
    * @return returns true is instrument is a synth, otherwise return false
    */
    virtual bool isSynth()
    {
        return false;
    }

    virtual bool acceptsMidiInput()
    {
        return false;
    }

    /** Called by Lattice to determine the type of module to load.
    * @return returns the type of module
    */
    virtual int getModuleType()
    {
        return ModuleType::AudioProcessor::uncategorised;
    }


    /** Used to query the current value of a parameter - the names correspond to the names used
        in the createParameters() function
        * @return the current value of the parameter
        */
    float getParameter(std::string name)
    {
        if(parameterValues.count(name))
        {
            auto value = parameterValues.at(name).load() + automationValues.at(name).load();
            return std::clamp(value, parameterRanges.at(name).load().min, parameterRanges.at(name).load().max);
        }

        return 0;
    }

    void updateAutomationForParameters(float** buffer, int numChannels)
    {
        int inputs = getNumberOfInputChannels() + 1;

        for(int i = inputs ; i < numChannels ; i++)
        {
            if(isInputConnected(i))
            {
                const auto parameterName = parameters[i-inputs].parameterName;
                automationValues.at(parameterName) = buffer[i][0];
            }
        }
    }
    
    /** Call this to inform the host that a parameter has been updated.
       Be careful that you don't call this too often!
    * @param [in] parameterID The parameter in the host you wish to update
    * @param [in] newValue The value you wish to set the parameter to.
    */
    void updateHostParameter(const char* parameterID, float newValue)
    {
        if (hostParamCallback)
            hostParamCallback(parameterID, newValue);
    }
    

    
    /** Call this to inform the host that a parameter has been updated.
       Be careful that you don't call this too often!
    * @param [in] parameterID The parameter in the host you wish to update
    * @param [in] newValue The value you wish to set the parameter to.
    */
    void updateHostParameter(const char* parameterID, const char* newValue)
    {
        if (hostParamCallbackChar)
            hostParamCallbackChar(parameterID, newValue);
    }
    
    void registerHostParameterCallback(const std::function<void(const char*, float)>& func1, const std::function<void(const char*, const char*)>& func2)
    {
        hostParamCallback = func1;
        hostParamCallbackChar = func2;
    }
    
    void registerSampleLoadCallback(const std::function<void(const char*)>& func1)
    {
        sampleLoadCallback = func1;
    }
    
    void loadSamplePack(const char* filename)
    {
        if (sampleLoadCallback)
            sampleLoadCallback(filename);
    }
    
    void updateParameter(std::string name, float newValue)
    {
        parameterValues.at(name) = newValue;
    }
    
    /** this method will extract the parameter name, as defined in createParameters(), from the unique
     paramaterId assigned to each parameter by the host
    * @param [in] parameterId The long version of the parameter typically in the form of 'module parameter[ModuleName (NodeNum)]'
    * @return The parameter name as defined when creating the parameters
    */
    std::string getParameterNameFromId(std::string parameterId)
    {
        return parameterId.substr(parameterId.find_last_of("|") + 2);
    }

    /** override this method if you want to draw to the Lattice generic editor viewport
     * @return This should return a Valid SVG Xml string
    */
    virtual const char* getSVGXml() { return ""; }

    /** override this method and return true if you wish to enable drawing on the generic editor viewport. Note that you should only return true when you need the graphics updated. Leaving this permanently set to true will have a negative effect on performance.
     * @return This should return true is you wish the viewport to update
    */
    virtual bool canDraw()
    {
        return false;
    }

    /** Called by Lattice when a user right-clicks and module and select 'Show Info'
     * @return Text that will appear in a module's info window.
    */
    virtual const char* getDescription()
    {
        return "";
    }

    //==================================================================
    std::size_t getNumberOfChannels()
    {
        return channels.size();
    }

    std::size_t getNumberOfInputChannels()
    {
        int numInputs = 0;
        for(auto& c : channels)
            if(c.type == ChannelType::input)
                numInputs++;
        
        return numInputs;
    }
    
    std::size_t getNumberOfOutputChannels()
    {
        int numOutputs = getModuleType() >= 200 && getModuleType() < 300;
        for(auto& c : channels)
            if(c.type == ChannelType::output)
                numOutputs++;
        
        return numOutputs;
    }
    
    std::size_t getNumberOfParameters()
    {
        return parameters.size();
    }

    void addChannel(Channel c)
    {
        bool channelExists = false;
        for(auto& chan : channels)
        {
            if(chan.name == c.name)
            {
                channelExists = true;
                std::cout << "Can't add duplicate channels";
            }
        }
        
        
        if(channelExists == false)
        {
            channels.push_back(c);
            
            if (c.type == ChannelType::input)
            {
                connections[inCount] = { inCount , false };
                inCount++;
            }
        }
            

    }

    void addParameter(Parameter p)
    {
        parameters.push_back(p);
        parameterValues[p.parameterName].store(p.range.defaultValue);
        automationValues[p.parameterName].store(0.f);
        parameterRanges[p.parameterName].store(p.range);
    }

    Parameter* getParameters()
    {
        return parameters.data();
    }

    Channel* getChannels()
    {
        return channels.data();
    }

    struct Connection
    {
        int index = 0;
        bool isConnected = false;
        Connection() {}
        Connection(int i, bool c): index(i), isConnected(c){}
    };
    
    void setConnection(int index, bool isConnected)
    {
        connections[index].isConnected = isConnected;
        if(!isConnected)
        {
            auto it = automationValues.begin();
            std::advance(it, std::max(0, int(index-getNumberOfInputChannels())));
            automationValues.at(it->first) = 0;
        }
    }
    
    bool isInputConnected(std::size_t index)
    {
        return connections[index].isConnected;
    }

    struct AudioFileSamples{
        const float** data;
        int numChannels;
        int numSamples;
        AudioFileSamples() = default ;
        AudioFileSamples(const float** d, int c, int s)
        :data(d), numChannels(c), numSamples(s){}
    };
    
    void registerAudioFileCallback(AudioFileSamples(*func)( const char* filename))
    {
        audioFileSamplesCallback = func;
    }

    
    AudioFileSamples getSamplesFromFile(const char* filename)
    {
        if (audioFileSamplesCallback != nullptr)
            return audioFileSamplesCallback(filename);
        
        return AudioFileSamples();
    }

    void setVoiceNum(int vn) {
      voiceNum = vn;
    }

    int getVoiceNum() {
      return voiceNum;
    }


    std::vector<LatticeProcessorModule *> &getVoices() {
      return voices;
    }
     
private:
    std::vector<Channel> channels;
    int inCount = 0;
    Connection connections[24];
    std::vector<Parameter> parameters;
    int midiNoteNumber = 0;
    std::string nodeName;
    std::map<std::string, std::atomic<float>> parameterValues;
    std::map<std::string, std::atomic<float>> automationValues;
    std::map<std::string, std::atomic<Parameter::Range>> parameterRanges;
    AudioFileSamples(*audioFileSamplesCallback)(const char* channel);
    //std::function<void(const std::string& parameterID, float newValue)> paramCallback;
    std::function<void(const char*, float)> hostParamCallback;
    std::function<void(const char*)> sampleLoadCallback;
    std::function<void(const char*, const char*)> hostParamCallbackChar;
    int voiceNum = -1;
    std::vector<LatticeProcessorModule *> voices;
};

#ifdef JUCE_MAC
typedef LatticeProcessorModule* create_t();
typedef void destroy_t(LatticeProcessorModule*);
#else
typedef LatticeProcessorModule* create_t();
typedef void destroy_t(LatticeProcessorModule*);
#endif
