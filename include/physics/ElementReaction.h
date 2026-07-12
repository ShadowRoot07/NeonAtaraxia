#ifndef ELEMENT_REACTION_H
#define ELEMENT_REACTION_H

#include "physics/ParticlePool.h"

class ElementReaction {
public:
    // Clase utilitaria estática
    ElementReaction() = delete;
    ElementReaction(const ElementReaction&) = delete;
    ElementReaction& operator=(const ElementReaction&) = delete;

    static void ResolveInteractions(Particle& p1, Particle& p2) noexcept;
};

#endif
