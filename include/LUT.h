#pragma once

#include <iostream>
#include <map>

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
{75,0}
};

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
{75,0}
};

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
{75,0}
};

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
{6500,0}
};

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
{100,100}
};