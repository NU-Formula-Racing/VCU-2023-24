#include <iostream>
#include <map>
using namespace std;

// inverter temp-amp lookup table
map<int, float> italut{{0,230},
{30,230},
{35,230},
{40,220},
{45,210},
{50,190},
{55,170},
{60,140},
{65,60},
{70,0},
{75,0},
};

// motor temp-amp lookup table
map<int, float> mtalut{{0,230},
{30,230},
{35,230},
{40,220.8},
{45,211.6},
{50,190.9},
{55,154.1},
{60,115},
{65,60},
{70,20},
{75,0}
};

// motor rmp-amp lookup table
map<int, float> mralut{{0,230},
{500,230},
{1000,230},
{1500,230},
{2000,230},
{2500,230},
{3000,230},
{3500,205},
{4000,179.5},
{4500,161.3},
{5000,143.6},
{5500,125},
{6000,114.6},
{6500,105.8},
};

// throttle map (percentage to torque multiplier) lookup table
map<float, float> tmlut{{0,0},
{0.05,0.01},
{0.1,0.03},
{0.15,0.05},
{0.2,0.07},
{0.25,0.09},
{0.3,0.12},
{0.35,0.16},
{0.4,0.22},
{0.45,0.3},
{0.5,0.38},
{0.55,0.47},
{0.6,0.55},
{0.65,0.62},
{0.7,0.68},
{0.75,0.74},
{0.8,0.8},
{0.85,0.85},
{0.9,0.9},
{0.95,0.95},
{1,1},
};