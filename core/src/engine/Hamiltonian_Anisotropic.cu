#ifdef USE_CUDA

#define _USE_MATH_DEFINES
#include <cmath>

#include <Eigen/Dense>

#include <engine/Hamiltonian_Anisotropic.hpp>
#include <engine/Vectormath.hpp>
#include <data/Spin_System.hpp>

using std::vector;
using std::function;

using namespace Data;

namespace Engine
{
	Hamiltonian_Anisotropic::Hamiltonian_Anisotropic(
			scalarfield mu_s,
			intfield external_field_index, scalarfield external_field_magnitude, vectorfield external_field_normal,
			intfield anisotropy_index, scalarfield anisotropy_magnitude, vectorfield anisotropy_normal,
			std::vector<indexPairs> Exchange_indices, std::vector<scalarfield> Exchange_magnitude,
			std::vector<indexPairs> DMI_indices, std::vector<scalarfield> DMI_magnitude, std::vector<vectorfield> DMI_normal,
			std::vector<indexPairs> DD_indices, std::vector<scalarfield> DD_magnitude, std::vector<vectorfield> DD_normal,
			std::vector<indexQuadruplets> quadruplet_indices, std::vector<scalarfield> quadruplet_magnitude,
			std::vector<bool> boundary_conditions
	) :
		Hamiltonian(boundary_conditions),
		mu_s(mu_s),
		external_field_index(external_field_index), external_field_magnitude(external_field_magnitude), external_field_normal(external_field_normal),
		anisotropy_index(anisotropy_index), anisotropy_magnitude(anisotropy_magnitude), anisotropy_normal(anisotropy_normal),
		Exchange_indices(Exchange_indices), Exchange_magnitude(Exchange_magnitude),
		DMI_indices(DMI_indices), DMI_magnitude(DMI_magnitude), DMI_normal(DMI_normal),
		DD_indices(DD_indices), DD_magnitude(DD_magnitude), DD_normal(DD_normal),
		Quadruplet_indices(quadruplet_indices), Quadruplet_magnitude(quadruplet_magnitude)
	{
		// Renormalize the external field from Tesla to whatever
		for (unsigned int i = 0; i < external_field_magnitude.size(); ++i)
		{
			this->external_field_magnitude[i] = this->external_field_magnitude[i] * Vectormath::MuB() * mu_s[i];
		}

		this->Update_Energy_Contributions();
	}

	void Hamiltonian_Anisotropic::Update_Energy_Contributions()
	{
		this->E = std::vector<std::pair<std::string, scalar>>(0);
		// External field
		if (this->external_field_index.size() > 0)
		{
			this->E.push_back({"Zeeman", 0});
			this->idx_zeeman = this->E.size()-1;
		}
		else this->idx_zeeman = -1;
		// Anisotropy
		if (this->anisotropy_index.size() > 0)
		{
			this->E.push_back({"Anisotropy", 0});
			this->idx_anisotropy = this->E.size()-1;
		}
		else this->idx_anisotropy = -1;
		// Exchange
		if (this->Exchange_indices[0].size() > 0)
		{
			this->E.push_back({"Exchange", 0});
			this->idx_exchange = this->E.size()-1;
		}
		else this->idx_exchange = -1;
		// DMI
		if (this->DMI_indices[0].size() > 0)
		{
			this->E.push_back({"DMI", 0});
			this->idx_dmi = this->E.size()-1;
		}
		else this->idx_dmi = -1;
		// Dipole-Dipole
		if (this->DD_indices[0].size() > 0)
		{
			this->E.push_back({"DD", 0});
			this->idx_dd = this->E.size()-1;
		}
		else this->idx_dd = -1;
		// Quadruplet
		if (this->Quadruplet_indices[0].size() > 0)
		{
			this->E.push_back({"Quadruplet", 0});
			this->idx_quadruplet = this->E.size()-1;
		}
		else this->idx_quadruplet = -1;
	}

	scalar Hamiltonian_Anisotropic::Energy(const vectorfield & spins)
	{
		scalar sum = 0;
		auto e = Energy_Array(spins);
		for (auto E : e) sum += E.second;
		return sum;
	}

	std::vector<std::pair<std::string, scalar>> Hamiltonian_Anisotropic::Energy_Contributions(const vectorfield & spins)
	{
		// Set to zero
		for (auto& pair : this->E) pair.second = 0;

		// External field
		int nfields=this->anisotropy_index.size();
		cu_E_Zeeman<<<(nfields+255)/256,256>>>(spins, nfields, this->external_field_index.data(), this->external_field_magnitude.data(), this->external_field_normal.data(), E[idx_zeeman].second);
		if (this->idx_zeeman >=0 ) E_Zeeman(spins, E[idx_zeeman].second);

		// Anisotropy
		if (this->idx_anisotropy >=0 ) E_Anisotropy(spins, E[idx_anisotropy].second);

		// Pairs
		//		Loop over periodicity
		for (int i_periodicity = 0; i_periodicity < 8; ++i_periodicity)
		{
			// Check if boundary conditions contain this periodicity
			if ((i_periodicity == 0)
				|| (i_periodicity == 1 && this->boundary_conditions[0])
				|| (i_periodicity == 2 && this->boundary_conditions[1])
				|| (i_periodicity == 3 && this->boundary_conditions[2])
				|| (i_periodicity == 4 && this->boundary_conditions[0] && this->boundary_conditions[1])
				|| (i_periodicity == 5 && this->boundary_conditions[0] && this->boundary_conditions[2])
				|| (i_periodicity == 6 && this->boundary_conditions[1] && this->boundary_conditions[2])
				|| (i_periodicity == 7 && this->boundary_conditions[0] && this->boundary_conditions[1] && this->boundary_conditions[2]))
			{
				//		Energies of this periodicity
				// Exchange
				if (this->idx_exchange >=0 ) E_Exchange(spins, Exchange_indices[i_periodicity], Exchange_magnitude[i_periodicity], E[idx_exchange].second);
				// DMI
				if (this->idx_dmi >=0 ) E_DMI(spins, DMI_indices[i_periodicity], DMI_magnitude[i_periodicity], DMI_normal[i_periodicity], E[idx_dmi].second);
				// DD
				if (this->idx_dd >=0 ) E_DD(spins, DD_indices[i_periodicity], DD_magnitude[i_periodicity], DD_normal[i_periodicity], E[idx_dd].second);
				// Quadruplet
				if (this->idx_quadruplet >=0 ) E_Quadruplet(spins, Quadruplet_indices[i_periodicity], Quadruplet_magnitude[i_periodicity], E[idx_quadruplet].second);
			}
		}

		// Return
		return this->E;
	}


	__global__ void cu_E_Zeeman(Vector3 *spins, int nfields, int *external_field_index, scalar *external_field_magnitude, Vector3 *external_field_normal, scalar *Energy)
	{
		for (int ifield = blockIdx.x * blockDim.x + threadIdx.x; ifield < nfields; ifield += blockDim.x * gridDim.x) 
		{
			int ispin = external_field_index[ifield];
			atomicAdd(Energy[ispin], - external_field_magnitude[ifield] * external_field_normal[ifield].dot(spins[ispin]));
		}
	}



	__global__ void Hamiltonian_Anisotropic::cu_E_Zeeman(Vector3 *spins, int *external_field_index, scalar *external_field_magnitude, Vector3 *external_field_normal, scalar *E , size_t N)
	{
		int idx = blockIdx.x * blockDim.x + threadIdx.x;
		if(idx < N)
		{
			atomicAdd(E[external_field_index[idx]], - external_field_magnitude[idx] * external_field_normal[idx].dot(spins[external_field_index[idx]]));
		}
	}
	void Hamiltonian_Anisotropic::E_Zeeman(const vectorfield & spins, scalar & Energy)
	{
		for (unsigned int i = 0; i < this->external_field_index.size(); ++i)
		{
			cu_scale<<<(n+1023)/1024, 1024>>>(spins.data(), this->external_field_index.data(), this->external_field_magnitude.data(), this->external_field_normal.data(), Energy, n);
		}
	}

	void Hamiltonian_Anisotropic::E_Anisotropy(const vectorfield & spins, scalar & Energy)
	{
		for (unsigned int i = 0; i < this->anisotropy_index.size(); ++i)
		{
			Energy -= this->anisotropy_magnitude[i] * std::pow(anisotropy_normal[i].dot(spins[anisotropy_index[i]]), 2.0);
		}
	}

	void Hamiltonian_Anisotropic::E_Exchange(const vectorfield & spins, indexPairs & indices, scalarfield & J_ij, scalar & Energy)
	{
		for (unsigned int i_pair = 0; i_pair < indices.size(); ++i_pair)
		{
			Energy -= J_ij[i_pair] * spins[indices[i_pair][0]].dot(spins[indices[i_pair][1]]);
		}
	}

	void Hamiltonian_Anisotropic::E_DMI(const vectorfield & spins, indexPairs & indices, scalarfield & DMI_magnitude, vectorfield & DMI_normal, scalar & Energy)
	{
		for (unsigned int i_pair = 0; i_pair < indices.size(); ++i_pair)
		{
			Energy -= DMI_magnitude[i_pair] * DMI_normal[i_pair].dot(spins[indices[i_pair][0]].cross(spins[indices[i_pair][1]]));
		}
	}

	void Hamiltonian_Anisotropic::E_DD(const vectorfield & spins, indexPairs & indices, scalarfield & DD_magnitude, vectorfield & DD_normal, scalar & Energy)
	{
		//scalar mult = -Utility::Vectormath::MuB()*Utility::Vectormath::MuB()*1.0 / 4.0 / M_PI; // multiply with mu_B^2
		scalar mult = 0.0536814951168; // mu_0*mu_B**2/(4pi*10**-30) -- the translations are in angstr�m, so the |r|[m] becomes |r|[m]*10^-10
		scalar result = 0.0;

		for (unsigned int i_pair = 0; i_pair < indices.size(); ++i_pair)
		{
			if (DD_magnitude[i_pair] > 0.0)
			{
				Energy -= mult / std::pow(DD_magnitude[i_pair], 3.0) *
					(3 * spins[indices[i_pair][1]].dot(DD_normal[i_pair]) * spins[indices[i_pair][0]].dot(DD_normal[i_pair]) - spins[indices[i_pair][0]].dot(spins[indices[i_pair][1]]));
			}

		}
	}// end DipoleDipole


	void Hamiltonian_Anisotropic::E_Quadruplet(const vectorfield & spins, indexQuadruplets & indices, scalarfield & magnitude, scalar & Energy)
	{
		for (unsigned int i_pair = 0; i_pair < indices.size(); ++i_pair)
		{
			Energy -= magnitude[i_pair] * (spins[indices[i_pair][0]].dot(spins[indices[i_pair][1]])) * (spins[indices[i_pair][2]].dot(spins[indices[i_pair][3]]));
		}
	}



	void Hamiltonian_Anisotropic::Gradient(const vectorfield & spins, vectorfield & gradient)
	{
		int nos = spins.size();
		// Loop over Spins
		for (int i = 0; i < nos; ++i)
		{
			gradient[i].setZero();
		}

		// External field
		Gradient_Zeeman(spins, gradient);

		// Anisotropy
		Gradient_Anisotropy(spins, gradient);

		// Pairs
		//		Loop over periodicity
		for (int i_periodicity = 0; i_periodicity < 8; ++i_periodicity)
		{
			// Check if boundary conditions contain this periodicity
			if ((i_periodicity == 0)
				|| (i_periodicity == 1 && this->boundary_conditions[0])
				|| (i_periodicity == 2 && this->boundary_conditions[1])
				|| (i_periodicity == 3 && this->boundary_conditions[2])
				|| (i_periodicity == 4 && this->boundary_conditions[0] && this->boundary_conditions[1])
				|| (i_periodicity == 5 && this->boundary_conditions[0] && this->boundary_conditions[2])
				|| (i_periodicity == 6 && this->boundary_conditions[1] && this->boundary_conditions[2])
				|| (i_periodicity == 7 && this->boundary_conditions[0] && this->boundary_conditions[1] && this->boundary_conditions[2]))
			{
				//		Gradients of this periodicity
				// Exchange
				this->Gradient_Exchange(spins, Exchange_indices[i_periodicity], Exchange_magnitude[i_periodicity], gradient);
				// DMI
				this->Gradient_DMI(spins, DMI_indices[i_periodicity], DMI_magnitude[i_periodicity], DMI_normal[i_periodicity], gradient);
				// DD
				this->Gradient_DD(spins, DD_indices[i_periodicity], DD_magnitude[i_periodicity], DD_normal[i_periodicity], gradient);
				// Quadruplet
				this->Gradient_Quadruplet(spins, Quadruplet_indices[i_periodicity], Quadruplet_magnitude[i_periodicity], gradient);
			}
		}

		// Triplet Interactions

		// Quadruplet Interactions
	}

	void Hamiltonian_Anisotropic::Gradient_Zeeman(const vectorfield & spins, vectorfield & gradient)
	{
		for (unsigned int i = 0; i < this->external_field_index.size(); ++i)
		{
			eff_field[external_field_index[i]] -= this->external_field_magnitude[i] * this->external_field_normal[i];
		}
	}

	void Hamiltonian_Anisotropic::Gradient_Anisotropy(const vectorfield & spins, vectorfield & gradient)
	{
		for (unsigned int i = 0; i < this->anisotropy_index.size(); ++i)
		{
			eff_field[anisotropy_index[i]] -= 2.0 * this->anisotropy_magnitude[i] * this->anisotropy_normal[i] * anisotropy_normal[i].dot(spins[anisotropy_index[i]]);
		}
	}

	void Hamiltonian_Anisotropic::Gradient_Exchange(const vectorfield & spins, indexPairs & indices, scalarfield & J_ij, vectorfield & gradient)
	{
		for (unsigned int i_pair = 0; i_pair < indices.size(); ++i_pair)
		{
			eff_field[indices[i_pair][0]] -= J_ij[i_pair] * spins[indices[i_pair][1]];
			eff_field[indices[i_pair][1]] -= J_ij[i_pair] * spins[indices[i_pair][0]];
		}
	}

	void Hamiltonian_Anisotropic::Gradient_DMI(const vectorfield & spins, indexPairs & indices, scalarfield & DMI_magnitude, vectorfield & DMI_normal, vectorfield & gradient)
	{
		for (unsigned int i_pair = 0; i_pair < indices.size(); ++i_pair)
		{
			eff_field[indices[i_pair][0]] -= DMI_magnitude[i_pair] * spins[indices[i_pair][1]].cross(DMI_normal[i_pair]);
			eff_field[indices[i_pair][1]] += DMI_magnitude[i_pair] * spins[indices[i_pair][0]].cross(DMI_normal[i_pair]);
		}
	}

	void Hamiltonian_Anisotropic::Gradient_DD(const vectorfield & spins, indexPairs & indices, scalarfield & DD_magnitude, vectorfield & DD_normal, vectorfield & gradient)
	{
		//scalar mult = Utility::Vectormath::MuB()*Utility::Vectormath::MuB()*1.0 / 4.0 / M_PI; // multiply with mu_B^2
		scalar mult = 0.0536814951168; // mu_0*mu_B**2/(4pi*10**-30) -- the translations are in angstr�m, so the |r|[m] becomes |r|[m]*10^-10
		
		for (unsigned int i_pair = 0; i_pair < indices.size(); ++i_pair)
		{
			if (DD_magnitude[i_pair] > 0.0)
			{
				scalar skalar_contrib = mult / std::pow(DD_magnitude[i_pair], 3.0);
				eff_field[indices[i_pair][0]] -= skalar_contrib * (3 * DD_normal[i_pair] * spins[indices[i_pair][1]].dot(DD_normal[i_pair]) - spins[indices[i_pair][1]]);
				eff_field[indices[i_pair][1]] -= skalar_contrib * (3 * DD_normal[i_pair] * spins[indices[i_pair][0]].dot(DD_normal[i_pair]) - spins[indices[i_pair][0]]);
			}
		}
	}//end Field_DipoleDipole


	void Hamiltonian_Anisotropic::Gradient_Quadruplet(const vectorfield & spins, indexQuadruplets & indices, scalarfield & magnitude, vectorfield & gradient)
	{
		for (unsigned int i_pair = 0; i_pair < indices.size(); ++i_pair)
		{
			eff_field[indices[i_pair][0]] -= magnitude[i_pair] * spins[indices[i_pair][1]] * (spins[indices[i_pair][2]].dot(spins[indices[i_pair][3]]));
			eff_field[indices[i_pair][1]] -= magnitude[i_pair] * spins[indices[i_pair][0]] *  (spins[indices[i_pair][2]].dot(spins[indices[i_pair][3]]));
			eff_field[indices[i_pair][2]] -= magnitude[i_pair] * (spins[indices[i_pair][0]].dot(spins[indices[i_pair][1]])) * spins[indices[i_pair][3]];
			eff_field[indices[i_pair][3]] -= magnitude[i_pair] * (spins[indices[i_pair][0]].dot(spins[indices[i_pair][1]])) * spins[indices[i_pair][2]];
		}
	}


	void Hamiltonian_Anisotropic::Hessian(const vectorfield & spins, MatrixX & hessian)
	{
		int nos = spins.size();

		// Set to zero
		// for (auto& h : hessian) h = 0;
		hessian.setZero();

		// Single Spin elements
		for (int alpha = 0; alpha < 3; ++alpha)
		{
			for (unsigned int i = 0; i < anisotropy_index.size(); ++i)
			{
				int idx = anisotropy_index[i];
				// scalar x = -2.0*this->anisotropy_magnitude[i] * std::pow(this->anisotropy_normal[i][alpha], 2);
				hessian(3*idx + alpha, 3*idx + alpha) += -2.0*this->anisotropy_magnitude[i]*std::pow(this->anisotropy_normal[i][alpha],2);
			}
		}

		// std::cerr << "calculated hessian" << std::endl;

		// // Spin Pair elements
		// for (int i_periodicity = 0; i_periodicity < 8; ++i_periodicity)
		// {
		// 	//		Check if boundary conditions contain this periodicity
		// 	if ((i_periodicity == 0)
		// 		|| (i_periodicity == 1 && this->boundary_conditions[0])
		// 		|| (i_periodicity == 2 && this->boundary_conditions[1])
		// 		|| (i_periodicity == 3 && this->boundary_conditions[2])
		// 		|| (i_periodicity == 4 && this->boundary_conditions[0] && this->boundary_conditions[1])
		// 		|| (i_periodicity == 5 && this->boundary_conditions[0] && this->boundary_conditions[2])
		// 		|| (i_periodicity == 6 && this->boundary_conditions[1] && this->boundary_conditions[2])
		// 		|| (i_periodicity == 7 && this->boundary_conditions[0] && this->boundary_conditions[1] && this->boundary_conditions[2]))
		// 	{
		// 		//		Loop over pairs of this periodicity
		// 		// Exchange
		// 		for (unsigned int i_pair = 0; i_pair < this->Exchange_indices[i_periodicity].size(); ++i_pair)
		// 		{
		// 			for (int alpha = 0; alpha < 3; ++alpha)
		// 			{
		// 				int idx_i = 3*Exchange_indices[i_periodicity][i_pair][0] + alpha;
		// 				int idx_j = 3*Exchange_indices[i_periodicity][i_pair][1] + alpha;
		// 				hessian(idx_i,idx_j) += -Exchange_magnitude[i_periodicity][i_pair];
		// 				hessian(idx_j,idx_i) += -Exchange_magnitude[i_periodicity][i_pair];
		// 			}
		// 		}
		// 		// DMI
		// 		for (unsigned int i_pair = 0; i_pair < this->DMI_indices[i_periodicity].size(); ++i_pair)
		// 		{
		// 			for (int alpha = 0; alpha < 3; ++alpha)
		// 			{
		// 				for (int beta = 0; beta < 3; ++beta)
		// 				{
		// 					int idx_i = 3*DMI_indices[i_periodicity][i_pair][0] + alpha;
		// 					int idx_j = 3*DMI_indices[i_periodicity][i_pair][1] + beta;
		// 					if ( (alpha == 0 && beta == 1) || (alpha == 1 && beta == 0) )
		// 					{
		// 						hessian(idx_i,idx_j) +=
		// 							DMI_magnitude[i_periodicity][i_pair] * DMI_normal[i_periodicity][i_pair][2];
		// 						hessian(idx_j,idx_i) +=
		// 							DMI_magnitude[i_periodicity][i_pair] * DMI_normal[i_periodicity][i_pair][2];
		// 					}
		// 					else if ( (alpha == 0 && beta == 2) || (alpha == 2 && beta == 0) )
		// 					{
		// 						hessian(idx_i,idx_j) +=
		// 							-DMI_magnitude[i_periodicity][i_pair] * DMI_normal[i_periodicity][i_pair][1];
		// 						hessian(idx_j,idx_i) +=
		// 							-DMI_magnitude[i_periodicity][i_pair] * DMI_normal[i_periodicity][i_pair][1];
		// 					}
		// 					else if ( (alpha == 1 && beta == 2) || (alpha == 2 && beta == 1) )
		// 					{
		// 						hessian(idx_i,idx_j) +=
		// 							DMI_magnitude[i_periodicity][i_pair] * DMI_normal[i_periodicity][i_pair][0];
		// 						hessian(idx_j,idx_i) +=
		// 							DMI_magnitude[i_periodicity][i_pair] * DMI_normal[i_periodicity][i_pair][0];
		// 					}
		// 				}
		// 			}
		// 		}
		// //		// Dipole-Dipole
		// //		for (unsigned int i_pair = 0; i_pair < this->DD_indices[i_periodicity].size(); ++i_pair)
		// //		{
		// //			// indices
		// //			int idx_1 = DD_indices[i_periodicity][i_pair][0];
		// //			int idx_2 = DD_indices[i_periodicity][i_pair][1];
		// //			// prefactor
		// //			scalar prefactor = 0.0536814951168
		// //				* this->mu_s[idx_1] * this->mu_s[idx_2]
		// //				/ std::pow(DD_magnitude[i_periodicity][i_pair], 3);
		// //			// components
		// //			for (int alpha = 0; alpha < 3; ++alpha)
		// //			{
		// //				for (int beta = 0; beta < 3; ++beta)
		// //				{
		// //					int idx_h = idx_1 + alpha*nos + 3 * nos*(idx_2 + beta*nos);
		// //					if (alpha == beta)
		// //						hessian[idx_h] += prefactor;
		// //					hessian[idx_h] += -3.0*prefactor*DD_normal[i_periodicity][i_pair][alpha] * DD_normal[i_periodicity][i_pair][beta];
		// //				}
		// //			}
		// //		}
		// 	}// end if periodicity
		// }// end for periodicity
	}

	// Hamiltonian name as string
	static const std::string name = "Anisotropic Heisenberg";
	const std::string& Hamiltonian_Anisotropic::Name() { return name; }
}

#endif