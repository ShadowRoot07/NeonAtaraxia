#ifndef ELEMENT_REACTION_H
#define ELEMENT_REACTION_H

#include "physics/ParticlePool.h"

// Forward declaration limpia
class ShadowAudio;

class ElementReaction {
public:
    static void ResolveInteractions(ParticlePool& pool);
    static void ResolveInteractions(ParticlePool& pool, ShadowAudio& audio);
    static void ResolveInteractions(Particle& p1, Particle& p2);
};

#endif // ELEMENT_REACTION_H
