#include "Threading.hpp"
#include "Handle_Signal.hpp"

#include "Interface_State.h"
#include "Interface_System.h"
#include "Interface_Chain.h"
#include "Interface_Configurations.h"
#include "Interface_Transitions.h"
#include "Interface_Simulation.h"
#include "Interface_Log.h"

#include <memory>

// Initialise Global Variables
std::shared_ptr<State> state;

// Main
int main(int argc, char ** argv)
{

	//--- Register SigInt
	signal(SIGINT, Utility::Handle_Signal::Handle_SigInt);
	
	//---------------------- file names ---------------------------------------------
	//--- Config Files
	const char * cfgfile = "input/input.cfg";
	// const char * cfgfile = "input/anisotropic/markus.cfg";
	// const char * cfgfile = "input/anisotropic/markus-paper.cfg";
	// const char * cfgfile = "input/anisotropic/kagome-spin-ice.cfg";
	// const char * cfgfile = "input/anisotropic/gideon-master-thesis-anisotropic.cfg";
	// const char * cfgfile = "input/isotropic/gideon-master-thesis-isotropic.cfg";
	// const char * cfgfile = "input/isotropic/daniel-master-thesis-isotropic.cfg";
	// const char * cfgfile = "input/gaussian/example-1.cfg";
	// const char * cfgfile = "input/gaussian/gideon-paper.cfg";
	//--- Data Files
	// std::string spinsfile = "input/anisotropic/achiral.txt";
	// std::string chainfile = "input/chain.txt";
	//-------------------------------------------------------------------------------
	
	//--- Initialise State
	std::shared_ptr<State> state = std::shared_ptr<State>(State_Setup(cfgfile), State_Delete);

	//---------------------- initialize spin_systems --------------------------------
	// Copy the system a few times
	Chain_Image_to_Clipboard(state.get());
	for (int i=1; i<7; ++i)
	{
		Chain_Insert_Image_After(state.get());
	}
	//-------------------------------------------------------------------------------
	
	//----------------------- spin_system_chain -------------------------------------
	// Parameters
	float dir[3] = { 0,0,1 };
	float pos[3] = { 0,0,0 };

	// Read Image from file
	//Configuration_from_File(state.get(), spinsfile, 0);
	// Read Chain from file
	//Chain_from_File(state.get(), chainfile);

	// First image is homogeneous with a Skyrmion at pos
	Configuration_Homogeneous(state.get(), dir, 0);
	Configuration_Skyrmion(state.get(), pos, 6.0, 1.0, -90.0, false, false, false, 0);
	// Last image is homogeneous
	Configuration_Homogeneous(state.get(), dir, Chain_Get_NOI(state.get())-1);

	// Create transition of images between first and last
	Transition_Homogeneous(state.get(), 0, Chain_Get_NOI(state.get())-1);

	// Update the Chain's Data'
	Chain_Update_Data(state.get());
	//-------------------------------------------------------------------------------

	//----------------------- LLG Iterations ----------------------------------------
	Simulation_PlayPause(state.get(), "LLG", "SIB");
	//-------------------------------------------------------------------------------

	return 0;
}