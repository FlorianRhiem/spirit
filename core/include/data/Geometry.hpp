#pragma once
#ifndef DATA_GEOMETRY_H
#define DATA_GEOMETRY_H

#include "Spirit_Defines.h"
#include <engine/Vectormath_Defines.hpp>

#include <vector>

namespace Data
{
    // TODO: replace that type with Eigen!
    typedef struct {
        scalar x, y, z;
    } vector3_t;
    typedef struct {
        scalar x, y;
    } vector2_t;

    typedef std::array<int, 4> tetrahedron_t;
    typedef std::array<int, 3> triangle_t;


    // A classifier for the kind of geometry defined here
	enum class GeometryType
	{
		Irregular,   // Single irregular basis cell
		Lattice,     // Lattice of irregular basis cells
		Rectilinear, // Lattice of irregular basis cells
		SC,          // Simple cubic lattice
		Hex2D,       // 2D Hexagonal lattice
		HCP,         // Hexagonal closely packed
		BCC,         // Body-centered cubic
		FCC          // Face-centered cubic
	};

    // Geometry contains all geometric information of a system
    class Geometry
    {
    public:
        // ---------- Constructor
        //  Build a regular lattice from a defined basis cell and translations
        Geometry(std::vector<Vector3> basis, std::vector<Vector3> translation_vectors,
            intfield n_cells, std::vector<Vector3> cell_atoms, scalar lattice_constant,
            intfield cell_atom_types);


        // ---------- Convenience Constructors

        // Rectilinear lattice with one atom per unit cell (i.e. simple cubic or simple rectangular)
        static Geometry Geometry_Rectilinear(intfield n_cells, Vector3 bounds_min, Vector3 bounds_max);
        // Hexagonal lattice
		static Geometry Geometry_Hex2D(intfield n_cells, scalar lattice_constant);


        // ---------- Convenience functions
        // Retrieve triangulation, if 2D
        const std::vector<triangle_t>&    triangulation(int n_cell_step=1);
        // Retrieve tetrahedra, if 3D
        const std::vector<tetrahedron_t>& tetrahedra(int n_cell_step=1);
        // Introduce disorder into the atom types
        // void disorder(scalar mixing);


        // ---------- Basic information set, which (in theory) defines everything

        // Basis vectors {a, b, c} of the unit cell
        std::vector<Vector3> basis;
        // Lattice Constant [Angstrom] (scales the translations)
        scalar lattice_constant;
        // Translation Vectors {ta, tb, tc} 
        std::vector<Vector3> translation_vectors;
        // Number of cells {na, nb, nc}
        intfield n_cells;
        // Number of spins per basic domain
        int n_cell_atoms;
        // Array of basis atom positions
        std::vector<Vector3> cell_atoms;
        // Atom types of the atoms in a unit cell:
        // type index 0..n or or vacancy (type < 0)
        std::vector<int> cell_atom_types;


        // ---------- Inferrable information

        // The kind of geometry
        GeometryType classifier;

        // Number of Spins total
        int nos;
        // Positions of all the atoms
        vectorfield positions;
        // Atom types of all the atoms: type index 0..n or or vacancy (type < 0)
        intfield atom_types;

        // Dimensionality of the points
        int dimensionality;
        // Center and Bounds
        Vector3 center, bounds_min, bounds_max;
        // Unit Cell Bounds
        Vector3 cell_bounds_min, cell_bounds_max;

        
    private:
        // Calculate and update the dimensionality of the points in this geometry
        void calculateDimensionality();
        // Calculate and update bounds of the System
        void calculateBounds();
        // Calculate and update unit cell bounds
        void calculateUnitCellBounds();

        // 
        std::vector<triangle_t>    _triangulation;
        std::vector<tetrahedron_t> _tetrahedra;
        
        // Temporaries to tell wether the triangulation or tetrahedra
        // need to be updated when the corresponding function is called
        int last_update_n_cell_step;
        intfield last_update_n_cells;
    };
}
#endif
