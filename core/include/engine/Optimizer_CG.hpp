#pragma once
#ifndef OPTIMIZER_CG_H
#define OPTIMIZER_CG_H

#include <engine/Optimizer.hpp>

namespace Engine
{
	/*
		Conjugate Gradient Optimizer
	*/
	class Optimizer_CG : public Optimizer
	{
	public:
		Optimizer_CG(std::shared_ptr<Engine::Method> method);

		// One Iteration
		void Iteration() override;

		// Optimizer name as string
		std::string Name() override;
		std::string FullName() override;
    };
}

#endif