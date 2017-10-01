// File:  project.c
// Author: JS
// Date:  9/9/17
// Purpose: ADC Experiment
// Hardware:  TM4C123 Tiva board

#include "VoltMeter.h"


// Main function.  Initializes the VoltMeter module then runs it.
int main()
{

	InitVoltMeter();
	
	while (1) 
	{		
		RunVoltMeter();
	}
	
}

	
