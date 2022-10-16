// the number of bits each field requires
#define X_BIT_DEPTH		11
#define Y_BIT_DEPTH		10
#define Z_BIT_DEPTH		11

// shift values
#define X_BIT_SHIFT		32 - X_BIT_DEPTH
#define Y_BIT_SHIFT		X_BIT_SHIFT - Y_BIT_DEPTH
#define Z_BIT_SHIFT		Y_BIT_SHIFT - Z_BIT_DEPTH

// the max value of each field
#define X_BIT_RES		(1 << X_BIT_DEPTH) - 1
#define Y_BIT_RES		(1 << Y_BIT_DEPTH) - 1
#define Z_BIT_RES		(1 << Z_BIT_DEPTH) - 1

// the bit mask
#define X_BIT_MASK		X_BIT_RES << X_BIT_SHIFT
#define Y_BIT_MASK		Y_BIT_RES << Y_BIT_SHIFT
#define Z_BIT_MASK		Z_BIT_RES << Z_BIT_SHIFT
