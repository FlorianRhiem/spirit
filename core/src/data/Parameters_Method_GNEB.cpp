#include <data/Parameters_Method_GNEB.hpp>

namespace Data
{
	Parameters_Method_GNEB::Parameters_Method_GNEB(std::string output_folder, std::array<bool,4> save_output, scalar force_convergence, long int n_iterations, long int n_iterations_log, scalar spring_constant, int n_E_interpolations) :
		Parameters_Method(output_folder, save_output, force_convergence, n_iterations, n_iterations_log), spring_constant(spring_constant), n_E_interpolations(n_E_interpolations)
	{
	}
}