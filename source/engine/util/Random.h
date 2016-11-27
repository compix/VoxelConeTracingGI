#pragma once

#include <random>

/**
Uses the std::mt19937 random number generator and uniform distributions.
*/
class Random
{
public:
    static void randomize();

    static void seed(unsigned int seed) { m_rng.seed(seed); }

    /**
    Returns a random int between start (included) and end(included).
    */
    static int getInt(int start, int end);


    /**
    Returns a random float in [start, end).
    */
    static float getFloat(float start, float end);

private:
    static std::mt19937 m_rng;
};
