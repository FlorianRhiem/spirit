#pragma once
#ifndef DATA_PARAMETERS_METHOD_LLG_H
#define DATA_PARAMETERS_METHOD_LLG_H

#include <random>
#include <vector>

#include "Core_Defines.h"
#include <engine/Vectormath_Defines.hpp>
#include <data/Parameters_Method.hpp>

namespace Data
{
	// LLG_Parameters contains all LLG information about the spin system
	class Parameters_Method_LLG : public Parameters_Method
	{
	public:
		// Constructor
		Parameters_Method_LLG(std::string output_folder, std::array<bool,6> save_output, scalar force_convergence, long int n_iterations, long int n_iterations_log,
			int seed_i, scalar temperature_i, scalar damping_i, scalar time_step_i, bool renorm_sd_i,
			scalar stt_magnitude_i, Vector3 stt_polarisation_normal_i);

		//PRNG Seed
		const int seed;
		// --------------- Different distributions ------------
		std::mt19937 prng;
		std::uniform_real_distribution<scalar> distribution_real;
		std::uniform_real_distribution<scalar> distribution_minus_plus_one;
		std::uniform_int_distribution<int> distribution_int;

		// Temperature [K]
		scalar temperature;
		// Damping
		scalar damping;
		// Time step per iteration
		scalar dt;
		// whether to renormalize spins after every SD iteration
		bool renorm_sd = 1;
		// Whether to save an archive of spin configurations
		bool save_output_archive;
		// Whether to save a single spin configurations
		bool save_output_single;

		// spin-transfer-torque parameter (prop to injected current density)
		scalar stt_magnitude;
		// spin_current polarisation normal vector
		Vector3 stt_polarisation_normal;
	};
}
#endif