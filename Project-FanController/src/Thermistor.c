// File:  Thermistor.h
// Author: JSpicer
// Date:  10/20/17
// Purpose: Thermistor utilities
// Hardware:  TM4C123 Tiva board

#include <stdint.h>
#include "Thermistor.h"

// Use the following forumla in the Vishay Excel spreadsheet to convert
//	the temperature to the ADC output.
// =CONCATENATE(TEXT(40960000/(C30+10000),"###"),",")

// Fahrenheit table:  0-212 degrees
static const uint32_t TempData_Fahrenheit[] = {	
	432,	
	444,
	457,
	470,
	483,
	496,
	510,
	524,
	538,
	553,
	568,
	583,
	598,
	614,
	630,
	646,
	662,
	679,
	696,
	713,
	731,
	749,
	767,
	785,
	804,
	823,
	842,
	862,
	881,
	901,
	921,
	942,
	963,
	983,
	1005,
	1026,
	1048,
	1070,
	1092,
	1114,
	1136,
	1159,
	1182,
	1205,
	1228,
	1252,
	1275,
	1299,
	1323,
	1347,
	1371,
	1395,
	1420,
	1445,
	1469,
	1494,
	1519,
	1544,
	1569,
	1594,
	1619,
	1644,
	1669,
	1695,
	1720,
	1745,
	1771,
	1796,
	1821,
	1847,
	1872,
	1897,
	1923,
	1948,
	1973,
	1998,
	2023,
	2048,
	2073,
	2098,
	2122,
	2147,
	2171,
	2196,
	2220,
	2244,
	2268,
	2292,
	2316,
	2339,
	2363,
	2386,
	2409,
	2432,
	2455,
	2477,
	2500,
	2522,
	2544,
	2566,
	2587,
	2609,
	2630,
	2651,
	2672,
	2693,
	2713,
	2733,
	2753,
	2773,
	2793,
	2812,
	2831,
	2850,
	2869,
	2887,
	2906,
	2924,
	2941,
	2959,
	2976,
	2994,
	3011,
	3027,
	3044,
	3060,
	3076,
	3092,
	3108,
	3123,
	3138,
	3154,
	3168,
	3183,
	3197,
	3212,
	3226,
	3239,
	3253,
	3266,
	3279,
	3292,
	3305,
	3318,
	3330,
	3342,
	3354,
	3366,
	3378,
	3389,
	3401,
	3412,
	3423,
	3434,
	3444,
	3455,
	3465,
	3475,
	3485,
	3495,
	3504,
	3514,
	3523,
	3532,
	3541,
	3550,
	3559,
	3568,
	3576,
	3584,
	3593,
	3601,
	3609,
	3616,
	3624,
	3631,
	3639,
	3646,
	3653,
	3660,
	3667,
	3674,
	3681,
	3687,
	3694,
	3700,
	3706,
	3712,
	3718,
	3724,
	3730,
	3736,
	3742,
	3747,
	3752,
	3758,
	3763,
	3768,
	3773,
	3778,
	3783,
	3788,
	3793,
	3798,
	3802,
	3807,
	3811,
	3815,
	3820,
	3824,
	3828,
	3832,
	3836
};

// Celsius table:  -17 to 100 degrees
static const uint32_t TempData_Celsius[] = {	
	449,
	472,
	496,
	521,
	547,
	574,
	601,
	630,
	659,
	689,
	720,
	752,
	785,
	819,
	854,
	889,
	925,
	963,
	1000,
	1039,
	1078,
	1118,
	1159,
	1200,
	1242,
	1285,
	1328,
	1371,
	1415,
	1459,
	1504,
	1549,
	1594,
	1639,
	1685,
	1730,
	1776,
	1821,
	1867,
	1913,
	1958,
	2003,
	2048,
	2093,
	2137,
	2181,
	2225,
	2268,
	2311,
	2353,
	2395,
	2436,
	2477,
	2517,
	2557,
	2596,
	2634,
	2672,
	2709,
	2745,
	2781,
	2816,
	2850,
	2884,
	2916,
	2949,
	2980,
	3011,
	3041,
	3070,
	3098,
	3126,
	3154,
	3180,
	3206,
	3231,
	3256,
	3279,
	3303,
	3325,
	3347,
	3369,
	3389,
	3410,
	3429,
	3448,
	3467,
	3485,
	3503,
	3520,
	3536,
	3552,
	3568,
	3583,
	3597,
	3612,
	3625,
	3639,
	3652,
	3664,
	3677,
	3688,
	3700,
	3711,
	3722,
	3732,
	3743,
	3752,
	3762,
	3771,
	3780,
	3789,
	3798,
	3806,
	3814,
	3821,
	3829,
	3836
};


// Performs a binary search on a temperature table.
static int Search(uint32_t value, const uint32_t* array, int startIndex, int endIndex)
{
	//assert(startIndex >= 0 && endIndex >= 0);
	
	int foundIndex = -1;
		
	if (value <= array[startIndex]) {
		foundIndex = startIndex;
	}
	else if (value >= array[endIndex]) {
		foundIndex = endIndex;
	}
	else {
		while (foundIndex == -1) {
			
			int midIndex = startIndex + ((endIndex - startIndex) / 2);
			
			if (midIndex == startIndex) {
				foundIndex = startIndex;
			}
			else if (value == array[midIndex]) {
				foundIndex = midIndex;
			}
			else if (value < array[midIndex]) {
				endIndex = midIndex;
			}
			else {
				startIndex = midIndex;
			}
		}
	}
	
	return foundIndex;
}

// Get the temperature in the give scale based on the ADC output.
int Therm_GetTemperature(uint32_t adcSample, ThermScale_t scale) 
{
	int startIndex = 0;
	int endIndex;
	const uint32_t* array;
	int arraySize;
	
	// Choose which temperature data table to search.
	if (scale == THERM_FAHRENHEIT) {
		array = TempData_Fahrenheit;
		arraySize = sizeof(TempData_Fahrenheit);
	}
	else {
		array = TempData_Celsius;
		arraySize = sizeof(TempData_Celsius);
	}
	
	endIndex = (arraySize / sizeof(uint32_t)) - 1;
	
	// Search for the table index that is less than or equal to the ADC input. 
	int foundIndex = Search(adcSample, array, startIndex, endIndex);
	
	// Round up to the next temperature if necessary.
	if (foundIndex + 1 <= endIndex) {

		uint32_t lowSample = array[foundIndex];
		uint32_t highSample = array[foundIndex + 1];
		uint32_t midSample = lowSample + ((highSample - lowSample) / 2);
		if (adcSample > midSample) {
			foundIndex++;
		}
	}

	// Convert the table index to degrees.
	int temperature = foundIndex;
	if (scale == THERM_CELSIUS) {
		temperature -= 17;
	}
	
	return temperature;
}	



