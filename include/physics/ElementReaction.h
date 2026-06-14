#ifndef ELEMENT_REACTION_H
#define ELEMENT_REACTION_H

#include "physics/ParticlePool.h"

class ElementReaction {
public:
    // Procesa la interacción mutua entre dos partículas activas basándose en su cercanía
    static void ResolveInteractions(Particle& p1, Particle& p2);
};

#endif // ELEMENT_REACTION_H
