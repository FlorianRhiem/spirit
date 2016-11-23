#include "engine/Vectormath.hpp"

#include <stdio.h>

// Returns the Bohr Magneton [meV / T]
scalar Engine::Vectormath::MuB()
{
	return 0.057883817555;
}

// Returns the Boltzmann constant [meV / K]
scalar Engine::Vectormath::kB()
{
	return 0.08617330350;
}

///////////// Quick Debug Method that is used occasionally to print some arrays -> move to Utility_IO?!
void Engine::Vectormath::Array_to_Console(const scalar *array, const int length) {
	std::cout << std::endl;
	for (int i = 0; i < length; ++i) {
		std::cout << array[i] << ' ';
	}
	std::cout << std::endl;
};//end Array_to_Console

void Engine::Vectormath::Array_to_Console(const int *array, const int length) {
	std::cout << std::endl;
	for (int i = 0; i < length; ++i) {
		std::cout << array[i] << ' ';
	}
	std::cout << std::endl;
};//end Array_to_Console
  /////////////////////


std::vector<scalar> Engine::Vectormath::scalar_product(std::vector<Vector3> vector_v1, std::vector<Vector3> vector_v2)
{
    std::vector<scalar> result(vector_v1.size());
    for (unsigned int i=0; i<vector_v1.size(); ++i)
    {
        result[i] = vector_v1[i].dot(vector_v2[i]);
    }
	return result;
}


void Engine::Vectormath::Normalize(std::vector<Vector3> & spins)
{
	scalar norm = 0;
	for (int i = 0; i < spins.size(); ++i)
	{
		for (int dim = 0; dim < 3; ++dim)
		{
			norm += std::pow(spins[i][dim], 2);
		}
	}
	scalar norm1 = 1.0 / norm;
	for (int i = 0; i < spins.size(); ++i)
	{
		spins[i] *= norm1;
	}
}



scalar Engine::Vectormath::dist_greatcircle(Vector3 v1, Vector3 v2)
{
	scalar r = v1.dot(v2);

	// Prevent NaNs from occurring
	r = std::fmax(-1.0, std::fmin(1.0, r));

	// Greatcircle distance
	return std::acos(r);
}


scalar Engine::Vectormath::dist_geodesic(std::vector<Vector3> v1, std::vector<Vector3> v2)
{
	scalar dist = 0;
	for (unsigned int i = 0; i < v1.size(); ++i)
	{
		dist = dist + pow(dist_greatcircle(v1[i], v2[i]), 2);
	}
	return sqrt(dist);
}



void Engine::Vectormath::Project_Reverse(std::vector<Vector3> v1, std::vector<Vector3> v2)
{
	// Get the scalar product of the vectors
	scalar v1v2 = 0.0;
	for (unsigned int i = 0; i < v1.size(); ++i)
	{
		v1v2 += v1[i].dot(v2[i]);
	}

	// Take out component in direction of v2
	for (int i = 0; i < v1.size(); ++i)
	{
		v1[i] -= 2 * v1v2 * v2[i];
	}
}



/*
	Calculates the 'tangent' vectors, i.e.in crudest approximation the difference between an image and the neighbouring
*/
void Engine::Vectormath::Tangents(std::vector<std::shared_ptr<std::vector<Vector3>>> configurations, std::vector<scalar> energies, std::vector<std::vector<Vector3>> & tangents)
{
	int noi = configurations.size();
	int nos = (*configurations[0]).size();

	for (int idx_img = 0; idx_img < noi; ++idx_img)
	{
		auto& image = *configurations[idx_img];

		// First Image
		if (idx_img == 0)
		{
			auto& image_plus = *configurations[idx_img + 1];

			//tangents = IMAGES_LAST(idx_img + 1, :, : ) - IMAGES_LAST(idx_img, :, : );
			for (int i = 0; i < nos; ++i)
			{
				tangents[idx_img][i] = image_plus[i] - image[i];
			}
		}
		// Last Image
		else if (idx_img == noi - 1)
		{
			auto& image_minus = *configurations[idx_img - 1];

			//tangents = IMAGES_LAST(idx_img, :, : ) - IMAGES_LAST(idx_img - 1, :, : );
			for (int i = 0; i < nos; ++i)
			{
				tangents[idx_img][i] = image[i] - image_minus[i];
			}
		}
		// Images Inbetween
		else
		{
			auto& image_plus = *configurations[idx_img + 1];
			auto& image_minus = *configurations[idx_img - 1];

			// Energies
			scalar E_mid = 0, E_plus = 0, E_minus = 0;
			E_mid = energies[idx_img];
			E_plus = energies[idx_img + 1];
			E_minus = energies[idx_img - 1];

			// Vectors to neighbouring images
			std::vector<Vector3> t_plus(nos), t_minus(nos);
			for (int i = 0; i < nos; ++i)
			{
				t_plus[i] = image_plus[i] - image[i];
				t_minus[i] = image[i] - image_minus[i];
			}

			// Near maximum or minimum
			if ((E_plus < E_mid && E_mid > E_minus) || (E_plus > E_mid && E_mid < E_minus))
			{
				// Get a smooth transition between forward and backward tangent
				scalar E_max = std::fmax(std::abs(E_plus - E_mid), std::abs(E_minus - E_mid));
				scalar E_min = std::fmin(std::abs(E_plus - E_mid), std::abs(E_minus - E_mid));

				if (E_plus > E_minus)
				{
					//tangents = t_plus*E_max + t_minus*E_min;
					for (int i = 0; i < nos; ++i)
					{
						tangents[idx_img][i] = t_plus[i] * E_max + t_minus[i] * E_min;
					}
				}
				else
				{
					//tangents = t_plus*E_min + t_minus*E_max;
					for (int i = 0; i < nos; ++i)
					{
						tangents[idx_img][i] = t_plus[i] * E_min + t_minus[i] * E_max;
					}
				}
			}
			// Rising slope
			else if (E_plus > E_mid && E_mid > E_minus)
			{
				//tangents = t_plus;
				for (int i = 0; i < nos; ++i)
				{
					tangents[idx_img][i] = t_plus[i];
				}
			}
			// Falling slope
			else if (E_plus < E_mid && E_mid < E_minus)
			{
				//tangents = t_minus;
				for (int i = 0; i < nos; ++i)
				{
					tangents[idx_img][i] = t_minus[i];
				}
			}
			// No slope(constant energy)
			else
			{
				//tangents = t_plus + t_minus;
				for (int i = 0; i < nos; ++i)
				{
					tangents[idx_img][i] = t_plus[i] + t_minus[i];
				}
			}

		}

		// Project tangents onto normal planes of spin vectors to make them actual tangents
		//Project_Orthogonal(tangents[idx_img], configurations[idx_img]);
		scalar v1v2 = 0.0;
		for (int i = 0; i < nos; ++i)
		{
			// Get the scalar product of the vectors
			tangents[idx_img][i] -= tangents[idx_img][i].dot(image[i]) * image[i];
		}

		// Normalise in 3N - dimensional space
		Normalize(tangents[idx_img]);

	}// end for idx_img
}// end Tangents