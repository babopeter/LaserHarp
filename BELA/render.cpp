/*
____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
	Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
	Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/

#include "MassSpringDamper.h"
#include "String.h"
#include "Plectrum.h"

#include <Bela.h>
#include <cmath>
#include <stdio.h>
#include <cstdlib>

#define ACCEL_BUF_SIZE 1
#define ACCEL_BUF_SIZE2 1
#define ACCEL_BUF_SIZE3 1
#define ACCEL_BUF_SIZE4 1
#define NUMBER_OF_STRINGS 4

// PENTATONIC SCALE
//float gMidinotes[NUMBER_OF_STRINGS] = {40,45,50,55,57,60,62,64,67};
float gMidinotes[NUMBER_OF_STRINGS] = { 45, 62, 55, 67 };

int run = 0;

float gInverseSampleRate;

float out_gain = 1.0f;

float filter = 0;
int accelPin_x = 0;
int accelPin_y = 1;
int accelPin_z = 2;
int accelPin_w = 3;

float gPhase = 0.0;

MassSpringDamper msd = MassSpringDamper(1, 0.1, 10);// (10,0.001,10);
String strings[NUMBER_OF_STRINGS];
Plectrum plectrums[NUMBER_OF_STRINGS];

float gPlectrumDisplacement = 0;
int gCount = 0;
float gAccel_x[ACCEL_BUF_SIZE] = { 0 };
float gAccel_y[ACCEL_BUF_SIZE2] = { 0 };
float gAccel_z[ACCEL_BUF_SIZE3] = { 0 };
float gAccel_w[ACCEL_BUF_SIZE4] = { 0 };
int gAccelReadPtr = 0;
int gAccelReadPtr2 = 0;
int gAccelReadPtr3 = 0;
int gAccelReadPtr4 = 0;
float lastAccel = 1;
float lastAccel2 = 1;
float lastAccel3 = 1;
float lastAccel4 = 1;
float accel_x = 0;
float accel_y = 0;
float accel_z = 0;
float accel_w = 0;

// DC BLOCK BUTTERWORTH
// Coefficients for 100hz high-pass centre frequency

float a0_l = 0.9899759179893742;
float a1_l = -1.9799518359787485;
float a2_l = 0.9899759179893742;
float a3_l = -1.979851353142371;
float a4_l = 0.9800523188151258;

float a0_r = a0_l;
float a1_r = a1_l;
float a2_r = a2_l;
float a3_r = a3_l;
float a4_r = a4_l;

float x1_l = 0;
float x2_l = 0;
float y1_l = 0;
float y2_l = 0;

float x1_r = 0;
float x2_r = 0;
float y1_r = 0;
float y2_r = 0;

bool setup(BelaContext *context, void *userData)
{
	gInverseSampleRate = 1.0 / context->audioSampleRate;
	// initialise strings & plectrums
	for (int i = 0;i < NUMBER_OF_STRINGS;i++) {
		plectrums[i] = Plectrum();
		plectrums[i].setup(250, 0.25, 0.05);
		strings[i] = String();
		strings[i].setMidinote(gMidinotes[i]);
		//strings[i].wg_l.updateFilterCoeffs(0);
		//strings[i].wg_r.updateFilterCoeffs(0);
		//float spacing = 2.0 / (NUMBER_OF_STRINGS+1);
		strings[i].setGlobalPosition(1 /* -1 + spacing*(i+1) */);
		rt_printf("STRING %d // midinote: %f position: %f\n", i, gMidinotes[i], (strings[i].getGlobalPosition()));
	}
	return true;
}
// LEFT CHANNEL SOUND
float soundOut_l(float out)
{
	float out_l = 0;
	out_l += out;
	// APPLY DC-BLOCK FILTER TO OUTPUTS
	// LEFT CHANNEL
	float temp_in = out_l;
	/* compute result */
	out_l = a0_l * out_l + a1_l * x1_l + a2_l * x2_l - a3_l * y1_l - a4_l * y2_l;
	/* shift x1 to x2, sample to x1 */
	x2_l = x1_l;
	x1_l = temp_in;
	/* shift y1 to y2, result to y1 */
	y2_l = y1_l;
	y1_l = out_l;
	float temp_l = out_l;
	out_l = filter * (out_l + temp_l);
	return out_l;
}
// RIGHT CHANNEL SOUND
float soundOut_r(float out)
{
	float out_r = 0;
	float temp_in;
	out_r += out;
	// APPLY DC-BLOCK FILTER TO OUTPUTS
	// RIGHT CHANNEL
	temp_in = out_r;
	/* compute result */
	out_r = a0_r * out_r + a1_r * x1_r + a2_r * x2_r - a3_r * y1_r - a4_r * y2_r;
	/* shift x1 to x2, sample to x1 */
	x2_r = x1_r;
	x1_r = temp_in;
	/* shift y1 to y2, result to y1 */
	y2_r = y1_r;
	y1_r = out_r;
	float temp_r = out_r;
	out_r = filter * (out_r + temp_r);
	return out_r;
}

void render(BelaContext *context, void *userData)
{
	gCount++;

	for (unsigned int n = 0; n < context->audioFrames; ++n) {

		/*
		 *
		 * ACCELEROMETER DATA
		 *
		 */
		 // Read accelerometer data from analog input
		 // We are giving an initial pluck by setting lastAccel above.
		 //float accel_x = 0;
		if (n % 2) {
			float lightLevel1 = (float)context->analogIn[(n / 2) * 8 + accelPin_x];
			float lightLevel2 = (float)context->analogIn[(n / 2) * 8 + accelPin_y];
			float lightLevel3 = (float)context->analogIn[(n / 2) * 8 + accelPin_z];
			float lightLevel4 = (float)context->analogIn[(n / 2) * 8 + accelPin_w];
			if (lightLevel1 < 0.9) {
				accel_x = lightLevel1 * 2 - 1;	// 15800 - 28300 - 41500
				lastAccel = accel_x;
				strings[0].wg_l.updateFilterCoeffs(lightLevel1 / 2);
				strings[0].wg_r.updateFilterCoeffs(lightLevel1 / 2);
			}
			else {
				strings[0].wg_l.updateFilterCoeffs(0.5);
				strings[0].wg_r.updateFilterCoeffs(0.5);

			}
			if (lightLevel2 < 0.9) {
				accel_y = lightLevel2 * 2 - 1;	// 15800 - 28300 - 41500
				lastAccel2 = accel_y;
				strings[1].wg_l.updateFilterCoeffs(lightLevel2 / 2);
				strings[1].wg_r.updateFilterCoeffs(lightLevel2 / 2);
			}
			else {
				strings[1].wg_l.updateFilterCoeffs(0.5);
				strings[1].wg_r.updateFilterCoeffs(0.5);
			}
			if (lightLevel3 < 0.9) {
				accel_z = lightLevel3 * 2 - 1;	// 15800 - 28300 - 41500
				lastAccel3 = accel_z;
				strings[2].wg_l.updateFilterCoeffs(lightLevel3 / 2);
				strings[2].wg_r.updateFilterCoeffs(lightLevel3 / 2);
			}
			else {
				strings[2].wg_l.updateFilterCoeffs(0.5);
				strings[2].wg_r.updateFilterCoeffs(0.5);

			}
			if (lightLevel4 < 0.9) {
				accel_w = lightLevel4 * 2 - 1;	// 15800 - 28300 - 41500
				lastAccel4 = accel_w;
				strings[3].wg_l.updateFilterCoeffs(lightLevel4 / 2);
				strings[3].wg_r.updateFilterCoeffs(lightLevel4 / 2);
			}
			else {
				strings[3].wg_l.updateFilterCoeffs(0.5);
				strings[3].wg_r.updateFilterCoeffs(0.5);
			}
		}
		else {
			// grab previous value if !n%2
			accel_x = lastAccel;
			accel_y = lastAccel2;
			accel_z = lastAccel3;
			accel_w = lastAccel4;
		}
		//Variables for physics simulation
		float gravity = 0;
		float massPosition = 0;
		// Use this parameter to quickly adjust output gain
		float gain = 0.0015;	// 0.0015 is a good value or 12 strings

		//for loop for switch
		for (int s = 0; s < NUMBER_OF_STRINGS; s++) {

			//switch which gets physics simulation numbers
			switch (s)
			{
			case 0:
				//rt_printf("Case 0: %d\n",s);
				//rt_printf("Case 0");
				if (++gAccelReadPtr >= ACCEL_BUF_SIZE)
					gAccelReadPtr = 0;
				gAccel_x[gAccelReadPtr] = accel_x;

				for (int i = 0;i < ACCEL_BUF_SIZE;i++) {
					gravity = gAccel_x[(gAccelReadPtr - i + ACCEL_BUF_SIZE) % ACCEL_BUF_SIZE];
				}
				gravity /= ACCEL_BUF_SIZE;
				filter = map((float)context->analogIn[(n / 2) * 8 + accelPin_x], 0.1, 1, 0, 1);

				massPosition = (float)msd.update(gravity - gPlectrumDisplacement);
				break;

			case 1:
				//rt_printf("Case 1: %d\n",s);
				//rt_printf("Case 1");
				if (++gAccelReadPtr2 >= ACCEL_BUF_SIZE2)
					gAccelReadPtr2 = 0;
				gAccel_y[gAccelReadPtr2] = accel_y;
				for (int i = 0;i < ACCEL_BUF_SIZE2;i++) {
					gravity = gAccel_y[(gAccelReadPtr2 - i + ACCEL_BUF_SIZE2) % ACCEL_BUF_SIZE2];
				}
				gravity /= ACCEL_BUF_SIZE2;
				filter = map((float)context->analogIn[(n / 2) * 8 + accelPin_y], 0.1, 1, 0, 1);

				massPosition = (float)msd.update(gravity - gPlectrumDisplacement);
				break;

			case 2:
				//rt_printf("Case 2: %d\n",s);
				//rt_printf("Case 2");
				if (++gAccelReadPtr3 >= ACCEL_BUF_SIZE3)
					gAccelReadPtr3 = 0;
				gAccel_z[gAccelReadPtr3] = accel_z;
				for (int i = 0;i < ACCEL_BUF_SIZE3;i++) {
					gravity = gAccel_z[(gAccelReadPtr3 - i + ACCEL_BUF_SIZE3) % ACCEL_BUF_SIZE3];
				}
				gravity /= ACCEL_BUF_SIZE3;
				filter = map((float)context->analogIn[(n / 2) * 8 + accelPin_z], 0.1, 1, 0, 1);

				massPosition = (float)msd.update(gravity - gPlectrumDisplacement);
				break;

			case 3:
				//rt_printf("Case 3: %d\n",s);
				//rt_printf("Case 3");
				if (++gAccelReadPtr4 >= ACCEL_BUF_SIZE4)
					gAccelReadPtr4 = 0;
				gAccel_w[gAccelReadPtr4] = accel_w;
				for (int i = 0;i < ACCEL_BUF_SIZE4;i++) {
					gravity = gAccel_w[(gAccelReadPtr4 - i + ACCEL_BUF_SIZE4) % ACCEL_BUF_SIZE4];
				}
				gravity /= ACCEL_BUF_SIZE4;
				filter = map((float)context->analogIn[(n / 2) * 8 + accelPin_w], 0.1, 1, 0, 1);

				massPosition = (float)msd.update(gravity - gPlectrumDisplacement);
				break;
			}
			//rt_printf("%d\n", gravity);
		}

		gPlectrumDisplacement = 0;
		for (int s = 0; s < NUMBER_OF_STRINGS; s++) {

			// Perform smoothing (moving average) on acceleration value
			/*
			 *
			 * PHYSICS SIMULATION
			 *
			 */
			 // The horizontal force (which can be gravity if box is tipped on its side)
			 // is used as the input to a Mass-Spring-Damper model
			 // Plectrum displacement (i.e. when interacting with string) is included
			 //float test = map((float)context->analogIn[(n/2)*8+accelPin_x], 0.3, 1, 0, 1);
			float stringPosition = strings[s].getGlobalPosition();

			float plectrumForce = plectrums[s].update(massPosition, stringPosition);

			gPlectrumDisplacement += strings[s].getPlectrumDisplacement();

			// calculate panning based on string position (-1->left / 1->right)
			/*
			float panRight = map(stringPosition,1,-1,0.1,1);
			float panLeft = map(stringPosition,-1,1,0.1,1);
			panRight *= panRight;
			panLeft *= panLeft;
			*/
			context->audioOut[n * context->audioOutChannels + 1] = soundOut_l(strings[s].update(plectrumForce)*gain) * out_gain;
			context->audioOut[n * context->audioOutChannels + 0] = soundOut_r(strings[s].update(plectrumForce)*gain) * out_gain;

		}
	}
}
void cleanup(BelaContext *context, void *userData)
{}

/**
\example airharp/render.cpp

 * Physically modelled strings using waveguide junctions and mass-spring-dampers
 * controllable using an accelerometer
 *
 */
