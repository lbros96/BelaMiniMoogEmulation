//This file is an extension of the minimoog_complete example provided by Andrew McPherson as part of ECS7012P//

#include <Bela.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <libraries/Midi/Midi.h>
#include <cmath>

#include "ADSR.h"
#include "sawtooth.h"
#include "sine.h"
#include "filter.h"
#include "moogladder.h"

#include "polyBLEPoscillator.h"

// Browser-based GUI to adjust parameters
Gui gui;
GuiController controller;

// Device for handling MIDI messages
Midi gMidi;

// Name of the MIDI port to use. Run 'amidi -l' on the console to see a list.
// Typical values: 
//   "hw:0,0,0" for a virtual device (from the computer)
//   "hw:1,0,0" for a USB device plugged into the Bela board
const char* gMidiPort0 = "hw:1,0,0";

// Audio oscillator 
const int kNumOscillators = 3;
Sawtooth gOscillators[3];


float gOscillatorDetune = 0.0;
float gNoteFrequency = 0.0;

// LFO
Sine gVibratoLFO;
float gVibratoDepth = 0.0;

// ADSR envelope and overall amplitude
ADSR gAmplitudeADSR;
float gAmplitude = 1.0;

// Filter parameters
ADSR gFilterADSR;
float gFilterBaseline = 0.0;
float gFilterContour = 0.0;
float gFilterQ = 0.0;
float gFilterTracking = 0.0;

// Handling for multiple MIDI notes and pitchwheel
float gCurrentPitchBend = 0;

const int kMaxActiveNotes = 16;
int gActiveNotes[kMaxActiveNotes];
int gActiveNoteCount = 0;

//VARIABLES ADDED FOR THIS ADVANCED IMPLEMENTATION//
// Moog ladder filter
MoogLadder gLadderFilter;

// Global variable for the filter drive
float gFilterDrive = 1.0;

//polyBLEP oscillators for quasi-bandlimited waveforms
PolyBLEPOscillator gBLEPOscillators[3];

//global variables for implementing what oscillators to use
int gOscillator1Mode = SAW;
int gOscillator2Mode = SAW;
int gOscillator3Mode = SAW;

// Variables for implementing portamento/glide
float gPortamentoGlobalTime = 0.05;
int gPortamentoCurrentSample = 0;
float gPreviousFrequency = 0;

// Variable to hold the previous q value to check if a slider value has changed.
// To save efficiency on ladder filter as setting Q requires polynomial calculation
float gFilterPreviousQ = 0.0;


bool setup(BelaContext *context, void *userData)
{
	// Initialise the MIDI device
	if(gMidi.readFrom(gMidiPort0) < 0) {
		rt_printf("Unable to read from MIDI port %s\n", gMidiPort0);
		return false;
	}
	gMidi.writeTo(gMidiPort0);
	gMidi.enableParser(true);
	
	// Check if analog channels are enabled in 8 channel configuration, which 
	// produces twice as many audio frames as analog frames
	if(context->analogFrames != context->audioFrames / 2) {
		rt_printf("Error: this example needs analog enabled with 8 channels\n");
		return false;
	}
	
	// Initialise the oscillator
	for(int i = 0; i < kNumOscillators; i++){
		gOscillators[i].setSampleRate(context->audioSampleRate);
		gBLEPOscillators[i].setSampleRate(context->audioSampleRate);
	}
	
	// Set an inital mode for each oscillator
	gBLEPOscillators[0].setMode(gOscillator1Mode);
	gBLEPOscillators[1].setMode(gOscillator2Mode);
	gBLEPOscillators[2].setMode(gOscillator3Mode);
	
	// Initialise the ADSR, filter and LFO
	gAmplitudeADSR.setSampleRate(context->audioSampleRate);
	gFilterADSR.setSampleRate(context->audioSampleRate);
	gLadderFilter.setSampleRate(context->audioSampleRate);
	gVibratoLFO.setSampleRate(context->audioSampleRate);
	
	// Set up the GUI
	gui.setup(context->projectName);
	controller.setup(&gui, "Controller");	
	
	// Arguments: name, minimum, maximum, increment, default value
	controller.addSlider("Oscillator detune", 0.002, 0, 0.01, 0);
	
	controller.addSlider("VCA Attack time", 0.01, 0.001, 0.1, 0);
	controller.addSlider("VCA Decay time", 0.05, 0.01, 0.3, 0);
	controller.addSlider("VCA Sustain level", 0.3, 0, 1, 0);
	controller.addSlider("VCA Release time", 0.2, 0.001, 2, 0);
	
	controller.addSlider("Filter cutoff", 1000, 50, 5000, 0);
	controller.addSlider("Filter contour", 2000, 0, 5000, 0);
	controller.addSlider("Filter Q", 0.0, 0.0, 4.0, 0);
	controller.addSlider("Filter tracking", 0.33, 0, 1, 0);
	
	controller.addSlider("Filter Attack time", 0.03, 0.001, 0.2, 0);
	controller.addSlider("Filter Decay time", 0.2, 0.01, 1.0, 0);
	controller.addSlider("Filter Sustain level", 0.3, 0, 1, 0);
	controller.addSlider("Filter Release time", 0.2, 0.001, 2, 0);
	
	controller.addSlider("Vibrato frequency", 5, 1, 10, 0);
	
	controller.addSlider("Filter Drive", 1.0, 0.1, 5.0, 0);
	
	controller.addSlider("Oscillator 1 Mode", 1, 0, 2, 0);
	controller.addSlider("Oscillator 2 Mode", 1, 0, 2, 0);
	controller.addSlider("Oscillator 3 Mode", 1, 0, 2, 0);
	
	controller.addSlider("Portamento/Glide Time", 0.000, 0.001, 0.3, 0);

	return true;
}

// Update the frequency of the detuned oscillators
void updateOscillators(float frequency)
{
	gOscillators[0].setFrequency(frequency);
	gOscillators[1].setFrequency(frequency * (1.0 + gOscillatorDetune));
	gOscillators[2].setFrequency(frequency * (1.0 - gOscillatorDetune));	
	
	gBLEPOscillators[0].setFrequency(frequency);
	gBLEPOscillators[1].setFrequency(frequency * (1.0 + gOscillatorDetune));
	gBLEPOscillators[2].setFrequency(frequency * (1.0 - gOscillatorDetune));
	
}

// Calculate the frequency based on note and pitch bend
float calculateFrequency(int noteNumber, float pitchBend)
{
	float compositeNote = noteNumber + pitchBend;
	return powf(2.0, (compositeNote - 69)/12.0) * 440.0;
}

// MIDI note on received
void noteOn(int noteNumber, int velocity) 
{
	// Check if we have any note slots left
	if(gActiveNoteCount < kMaxActiveNotes) {
		// Keep track of this note, then play it
		gActiveNotes[gActiveNoteCount++] = noteNumber;
		
		//Set the current frequency as the previous frequency for Portamento
		gPreviousFrequency = gNoteFrequency;
		//Reset the current portamento sample
		gPortamentoCurrentSample = 0;
		
		// Map note number to frequency
		gNoteFrequency = calculateFrequency(noteNumber, gCurrentPitchBend);
		
		// Map velocity to amplitude on a decibel scale
		// float decibels = map(velocity, 1, 127, -40, 0);
		// gAmplitude = powf(10.0, decibels / 20.0);
	
		// For this Minimoog-style emulation, we don't have a velociy-sensitive keyboard
		gAmplitude = 1.0;
	
		// Start the ADSR, but only if this was the first note
		if(gActiveNoteCount == 1) {
			gAmplitudeADSR.trigger();
			gFilterADSR.trigger();
		}
	}
}

// MIDI note off received
void noteOff(int noteNumber)
{
	bool activeNoteChanged = false;
	
	// Go through all the active notes and remove any with this number
	for(int i = gActiveNoteCount - 1; i >= 0; i--) {
		if(gActiveNotes[i] == noteNumber) {
			// Found a match: is it the most recent note?
			if(i == gActiveNoteCount - 1) {
				activeNoteChanged = true;
			}
	
			// Move all the later notes back in the list
			for(int j = i; j < gActiveNoteCount - 1; j++) {
				gActiveNotes[j] = gActiveNotes[j + 1];
			}
			gActiveNoteCount--;
		}
	}
	
	if(gActiveNoteCount == 0) {
		// No notes left: stop the ADSR
		gAmplitudeADSR.release();
		gFilterADSR.release();
		
	}
	else if(activeNoteChanged) {
		// Update the frequency but don't retrigger
		int mostRecentNote = gActiveNotes[gActiveNoteCount - 1];
		
		//Update the previous note frequency for portamento
		gPreviousFrequency = gNoteFrequency;
		//Reset the current portamento sample
		gPortamentoCurrentSample = 0;
		
		gNoteFrequency = calculateFrequency(mostRecentNote, gCurrentPitchBend);
	}
}

// Handle pitch wheel messages
void pitchWheel(int lsb, int msb)
{
	// Pitch bend message has two data bytes: first is the least significant
	// bits, then the most significant bits. Put these together into a 14-bit
	// value.
	int wheelValue = lsb + (msb << 7);
	
	// The result is a value between 0 and 16383, with a centre value of
	// 8192 corresponding to no bend, and either end of the scale corresponding
	// to 2 semitones of difference. Calculate the fractional number of semitones.
	float bendSemitones = (wheelValue - 8192) * 2.0 / 8192;

	// Update the frequency, if a note is playing
	if(gActiveNoteCount > 0) {
		gNoteFrequency = calculateFrequency(gActiveNotes[gActiveNoteCount - 1], bendSemitones);
	}
		
	// Save the bend value for future notes
	gCurrentPitchBend = bendSemitones;	
}

void render(BelaContext *context, void *userData)
{
	// Get the slider values from the GUI
	gOscillatorDetune = controller.getSliderValue(0);
	
	gAmplitudeADSR.attackTime = controller.getSliderValue(1);
	gAmplitudeADSR.decayTime = controller.getSliderValue(2);
	gAmplitudeADSR.sustainLevel = controller.getSliderValue(3);
	gAmplitudeADSR.releaseTime = controller.getSliderValue(4);
	
	gFilterBaseline = controller.getSliderValue(5);
	gFilterContour = controller.getSliderValue(6);
	gFilterQ = controller.getSliderValue(7);
	gFilterTracking = controller.getSliderValue(8);
	
	gFilterADSR.attackTime = controller.getSliderValue(9);
	gFilterADSR.decayTime = controller.getSliderValue(10);
	gFilterADSR.sustainLevel = controller.getSliderValue(11);
	gFilterADSR.releaseTime = controller.getSliderValue(12);
	
	gVibratoLFO.setFrequency(controller.getSliderValue(13));
	
	gFilterDrive = controller.getSliderValue(14);
	
	// Collect slider values for setting oscillator mode
	gOscillator1Mode = controller.getSliderValue(15);
	gOscillator2Mode = controller.getSliderValue(16);
	gOscillator3Mode = controller.getSliderValue(17);
	
	gPortamentoGlobalTime = controller.getSliderValue(18);
	
	
	// This only needs to be updated when the slider changes
	if(gFilterQ != gFilterPreviousQ){
		gLadderFilter.setQ(gFilterQ);
	}
	
	gLadderFilter.setDrive(gFilterDrive);

	gBLEPOscillators[0].setMode(gOscillator1Mode);
	gBLEPOscillators[1].setMode(gOscillator2Mode);
	gBLEPOscillators[2].setMode(gOscillator3Mode);
	
	// At the beginning of each callback, look for available MIDI
	// messages that have come in since the last block
	while(gMidi.getParser()->numAvailableMessages() > 0) {
		MidiChannelMessage message;
		message = gMidi.getParser()->getNextChannelMessage();
		message.prettyPrint();		// Print the message data
		
		// A MIDI "note on" message type might actually hold a real
		// note onset (e.g. key press), or it might hold a note off (key release).
		// The latter is signified by a velocity of 0.
		if(message.getType() == kmmNoteOn) {
			int noteNumber = message.getDataByte(0);
			int velocity = message.getDataByte(1);
			
			// Velocity of 0 is really a note off
			if(velocity == 0) {
				noteOff(noteNumber);
			}
			else {
				noteOn(noteNumber, velocity);
			}
			
			rt_printf("Frequency: %f, Amplitude: %f\n", gNoteFrequency, gAmplitude);
		}
		else if(message.getType() == kmmNoteOff) {
			// We can also encounter the "note off" message type which is the same
			// as "note on" with a velocity of 0.
			int noteNumber = message.getDataByte(0);
			
			noteOff(noteNumber);
		}
		else if(message.getType() == kmmPitchBend) {
			int lsb = message.getDataByte(0);
			int msb = message.getDataByte(1);
		
			pitchWheel(lsb, msb);
		}
		else if(message.getType() == kmmControlChange) {
			int controller = message.getDataByte(0);
			int value = message.getDataByte(1);
			
			if(controller == 1) { // CC1 is the mod wheel
				gVibratoDepth = map(value, 0, 127, 0, 0.05);
			}
		}
	}

	// Now calculate the audio for this block
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		float value = 0;
		
		// Run the oscillator whenever a note is on
		if(gAmplitudeADSR.isActive()) {
			
			// Calculate the oscillator frequencies including the effect of vibrato 
			//float frequency = gNoteFrequency + gVibratoDepth * gNoteFrequency * gVibratoLFO.nextSample();

			// Calculate the start and end frequency for the portamento including the effect of vibrato
			float startFrequency = gPreviousFrequency + gVibratoDepth * gPreviousFrequency * gVibratoLFO.nextSample();
			float endFrequency = gNoteFrequency + gVibratoDepth * gNoteFrequency * gVibratoLFO.nextSample();
			
			//Portamento Section
			float outFrequency = endFrequency*(1-exp(-(gPortamentoCurrentSample)/(gPortamentoGlobalTime*context->audioSampleRate))) 
			+ startFrequency*exp(-(gPortamentoCurrentSample)/(gPortamentoGlobalTime*context->audioSampleRate));

			gPortamentoCurrentSample++;
			
			//Set frequency of oscillators including detune
			gBLEPOscillators[0].setFrequency(outFrequency);
			gBLEPOscillators[1].setFrequency(outFrequency * (1.0 + gOscillatorDetune));
			gBLEPOscillators[2].setFrequency(outFrequency * (1.0 - gOscillatorDetune));
			
			// Calculate the oscillator output
			value = 0;
			for(int i = 0; i < kNumOscillators; i++){
				value += gBLEPOscillators[i].nextSample();
			}
			value /= (float)kNumOscillators;
			
			// Calculate the filter cutoff frequency based on the 
			// ADSR and other filter parameters
			float filterCutoff = gFilterBaseline + gFilterContour * gFilterADSR.getNextValue();
			
			// Implement filter tracking off the cutoff frequency based on the note frequency
			filterCutoff += gFilterTracking * gNoteFrequency;
			
			//Update the filter cutoff
			gLadderFilter.setFrequency(filterCutoff);

			// Filter the oscillator output
			value = gLadderFilter.process(value);

			// Now apply the variable gain
			value *= 0.5 * gAmplitudeADSR.getNextValue();
		} 
		
		for(unsigned int ch = 0; ch < context->audioOutChannels; ++ch)
			audioWrite(context, n, ch, value);
	}
	
	
}

void cleanup(BelaContext *context, void *userData)
{
	
}

