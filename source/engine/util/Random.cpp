#include "Random.h"
#include <float.h>

std::mt19937 Random::m_rng;

void Random::randomize()
{
    std::random_device rd;
    m_rng.seed(rd());
}

int Random::getInt(int start, int end)
{
    std::uniform_int_distribution<int> distribution(start, end);
    return distribution(m_rng);
}

float Random::getFloat(float start, float end)
{
    std::uniform_real_distribution<float> distribution(start, end);
    return distribution(m_rng);
}
