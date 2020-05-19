/*
 * This is the config file.
 * Some important constants are defined here so they can be changed easily without digging through code
 */

// version string
#define VERSION             "v1.0"

// minimum time between menu inputs, in ms
#define TIME_BETWEEN_INPUT  250

// minecart velocity in pixels per second, depending on time (given in seconds)
#define CART_VELOCITY(time)         (32 + (time) / 60.f * 8)
#define CART_VELOCITY_SLOW(time)    (24 + (time) / 60.f * 6)
#define CART_VELOCITY_STAR(time)    (48 + (time) / 60.f * 12)

// vibration duration after collecting an object / hitting a bomb, in s
#define OBJECT_VIBRATION    0.05f
#define BOMB_VIBRATION      0.5f

// effects active time
#define EFFECT_SLOW_TIME    5.f
#define EFFECT_STAR_TIME    4.f

// objecs fall velocity, depending on time (given in seconds) - also gives velocity in pixels per second
// it increases by 8 every minute
#define FALL_SPEED(time)    (32 + (time) / 60.f * 8)

// time between spawning. This allows for precise fine tuning
#define TIME_BETWEEN_OBJECT_SPAWN_MIN 0.1f
#define TIME_BETWEEN_OBJECT_SPAWN_MAX 0.5f

// points granted for collecting treasure
#define NUGGET_POINTS         25
#define RUBY_POINTS           50
#define SAPPHIRE_POINTS      100
#define EMERALD_POINTS       200
#define DIAMOND_POINTS       500

// probability of the different object types.
#define NUGGET_PROBABILITY      20
#define RUBY_PROBABILITY        20
#define SAPPHIRE_PROBABILITY    15
#define EMERALD_PROBABILITY     10
#define DIAMOND_PROBABILITY      5
#define STAR_PROBABILITY         4
#define HEART_PROBABILITY        1
#define STONE_PROBABILITY       10
#define BOMB_PROBABILITY        15
