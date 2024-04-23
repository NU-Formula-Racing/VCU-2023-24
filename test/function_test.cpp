#include <iostream>
#include "assert.h"
#include <stdio.h>
#include <map>

int lookup(std::map<int, int> table, int key) {
	if (key < table.begin()->first) {
		return table.at(table.begin()->first);
	} else if (key > (prev(table.end()))->first) {
		return table.at(prev(table.end())->first);
	}
    std::map<int, int>::iterator it = table.find(key);
    if(it != table.end()) {
        return table.at(key);
    }
    else {
        it = table.begin();
        int prev = it->first;
        it++;
        for (it; it != table.end(); it++) {
            int curr = it->first;
            if(key > prev && key < curr) {
                return table.at(prev) - (table.at(prev) - table.at(curr))*(key-prev)/(curr-prev);
            }
            prev = curr;
        }
    }
}

int main() {
    // inverter temp-amp lookup table
    std::map<int, int> italut{{0,285},
                        {30,285},
                        {35,285},
                        {40,285},
                        {45,285},
                        {50,257},
                        {55,200},
                        {60,135},
                        {65,60},
                        {70,0},
                        {75,0}};

    // battery temp-amp lookup table
    std::map<int, int> btalut{{0,200},
                        {30,200},
                        {35,190},
                        {40,176},
                        {45,140},
                        {50,90},
                        {55,30},
                        {60,0},
                        {65,0},
                        {70,0},
                        {75,0}};

    // motor temp-torque lookup table
    std::map<int, int> mttlut{{0,230},
                        {30,230},
                        {35,230},
                        {40,220},
                        {45,205},
                        {50,180},
                        {55,142},
                        {60,100},
                        {65,55},
                        {70,20},
                        {75,0}};

    // motor rpm-torque lookup table
    std::map<int, int> mrtlut{{0,230},
                        {500,230},
                        {1000,230},
                        {1500,230},
                        {2000,230},
                        {2500,230},
                        {3000,230},
                        {3500,205},
                        {4000,180},
                        {4500,161},
                        {5000,140},
                        {5500,110},
                        {6000,63},
                        {6500,0}};

    // amp to torque conversion
// battery - torque = 9.5488 x voltage x amp / rpm
// inverter - torque = amp x 0.94

    // throttle map (percentage to torque multiplier) lookup table
    std::map<int, int> tmlut{{0,0},
                        {5,1},
                        {10,3},
                        {15,5},
                        {20,7},
                        {25,9},
                        {30,12},
                        {35,16},
                        {40,22},
                        {45,30},
                        {50,38},
                        {55,47},
                        {60,55},
                        {65,62},
                        {70,68},
                        {75,74},
                        {80,80},
                        {85,85},
                        {90,90},
                        {95,95},
                        {100,100}};
	
	assert(lookup(italut, 0) == 285);
    printf("inverter temp 0 -> %d amps\n", lookup(italut, 0));
	assert(lookup(italut, 15) == 285);
    printf("inverter temp 15 -> %d amps\n", lookup(italut, 15));
	assert(lookup(italut, 47) == 274);
    printf("inverter temp 47 -> %d amps\n", lookup(italut, 47));
	assert(lookup(italut, 56) == 187);
    printf("inverter temp 56 -> %d amps\n", lookup(italut, 56));
	assert(lookup(italut, 75) == 0);
    printf("inverter temp 75 -> %d amps\n", lookup(italut, 75));
	assert(lookup(italut, 76) == 0);
    printf("inverter temp 76 -> %d amps\n", lookup(italut, 76));
	printf("%f", 3/2);
    return 0;
}