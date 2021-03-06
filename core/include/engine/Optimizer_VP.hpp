#pragma once
#ifndef Optimizer_VP_H
#define Optimizer_VP_H

#include "Core_Defines.h"
#include <engine/Optimizer.hpp>

namespace Engine
{
	/*
		Velocity Projection Optimizer
	*/
	class Optimizer_VP : public Optimizer
	{
	public:
		Optimizer_VP(std::shared_ptr<Engine::Method> method);

		void Iteration() override;
		
		// Optimizer name as string
		std::string Name() override;
		std::string FullName() override;

	private:
		// "Mass of our particle" which we accelerate
		scalar m = 1.0;

		// Temporary Spins arrays
		std::vector<vectorfield> spins_temp;
		// Force in previous step [noi][nos]
		std::vector<vectorfield> force_previous;
		// Velocity in previous step [noi][nos]
		std::vector<vectorfield> velocity_previous;
		// Velocity used in the Steps [noi][nos]
		std::vector<vectorfield> velocity;
		// Projection of velocities onto the forces [noi]
		std::vector<scalar> projection;
		// |force|^2
		std::vector<scalar> force_norm2;
    };
}

#endif