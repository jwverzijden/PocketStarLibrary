/*
 * This is the config file.
 * Some important constants are defined here so they can be changed easily without digging through code
 */

// minecart velocity in pixels per second, depending on time (given in seconds)
// screen width is 96, so at the beginning, it takes about three seconds to drive from left to right
#define CART_VELOCITY(time)         (32 + (time) / 60.f * 8)

// vibration duration after collecting a gem / hitting a bomb, in seconds
#define OBJECT_VIBRATION    0.05f
#define BOMB_VIBRATION      0.5f

// gem and bomb fall velocity, depending in time (given in minutes) - also gives velocity in pixels per second
// it increases by 8 every minute
#define FALL_SPEED(time) (32 + (time) / 60 * 8)

// time between object spawning. This allows for precise fine tuning
#define TIME_BETWEEN_OBJECT_SPAWN_MIN 0.1f
#define TIME_BETWEEN_OBJECT_SPAWN_MAX 0.5f

// points granted for collecting gems
#define RUBY_POINTS      50
#define SAPPHIRE_POINTS 100
#define EMERALD_POINTS  200
#define DIAMOND_POINTS  500

