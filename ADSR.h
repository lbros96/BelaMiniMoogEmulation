// ECS7012P Music and Audio Programming
// School of Electronic Engineering and Computer Science
// Queen Mary University of London
// Spring 2020

// ADSR.h: header file for defining the ADSR class

class ADSR {
private:
	// ADSR state machine variables, used internally
	enum {
		kADSRStateOff = 0,
		kADSRStateAttack,
		kADSRStateDecay,
		kADSRStateSustain,
		kADSRStateRelease
	};

public:
	// Constructor
	ADSR();
	
	// Set the sample rate, used for all calculations
	void setSampleRate(float rate);
	
	// Start the envelope, going to the Attack state
	void trigger();
	
	// Stop the envelope, going to the Release state
	void release();
	
	// Calculate the next sample of output, changing the envelope
	// state as needed
	float getNextValue(); 
	
	// Indicate whether the envelope is active or not (i.e. in
	// anything other than the Off state
	bool isActive();
	
	// Destructor
	~ADSR();

public:
	// Adjustable parameters. Often the variables themselves are made
	// private or protected with methods to get and set them, but here
	// we will make them public for simplicity.
	
	float attackTime;
	float decayTime;
	float sustainLevel;
	float releaseTime;

private:
	// State variables, not accessible to the outside world
	int state;				// Current state of the ADSR
	float level;			// Current output level of the envelope
	float increment;		// Current rate of change of the envelope
	float sampleRate;		// Sample rate of the underlying system
};