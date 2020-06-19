/*
MoogLadder Filter class based on an implementation by Aaron Krajeski
This code is Unlicensed (i.e. public domain)
Aaron Krajeski has stated in an email: "That work is under no copyright. You may use it however you might like."
Source: http://song-swap.com/MUMT618/aaron/Presentation/demo.html
*/

// moogladder.h: header file for defining the moog ladder filter

// Thermal voltage (26 milliwats at room temperature)
#define VT 0.026
class MoogLadder{

public:
	// Constructor
	MoogLadder();
	
	// Constructor specifying a sample rate
	MoogLadder(float sampleRate);
	
	// Set the sample rate
	void setSampleRate(float rate);
	
	// Set the cutoff frequency 
	void setFrequency(float cutoff);
	
	// Set the filter Q
	void setQ(float resonance);
	
	void setDrive(float drive);
	
	// Reset previous history of filter
	void reset();
	
	// Calculate the next sample of output, changing the envelope
	// state as needed
	float process(float input); 
	
	// Destructor
	~MoogLadder();

private:

	// State variables, not accessible to the outside world
	float sampleRate_;	// Filter sample rate
	float cutoff_;		// Filter cutoff frequency
	float resonance_;	// Filter resonance
	double drive_;	// Filter drive.
	double gComp_;	// Compensation factor, used to decide how much of the input signal is added into the feedback loop.
	
	double state[5];// Array for holding the output of each stage of the ladder
	double delay[5];// Array for holding the delayed components
	double wc;		// The angular frequency of the cutoff.
	double g;		// A derived parameter for the cutoff frequency
	double gRes;	// A similar derived parameter for resonance.

	
};