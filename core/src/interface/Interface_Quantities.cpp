#include <interface/Interface_Quantities.h>
#include <interface/Interface_State.h>
#include <data/State.hpp>
#include <engine/Vectormath.hpp>

void Quantity_Get_Magnetization(State * state,  float m[3], int idx_image, int idx_chain)
{
    std::shared_ptr<Data::Spin_System> image;
    std::shared_ptr<Data::Spin_System_Chain> chain;
    from_indices(state, idx_image, idx_chain, image, chain);

    auto mag = Engine::Vectormath::Magnetization(*image->spins);

    for (int i=0; i<3; ++i) m[i] = (float)mag[i];
}