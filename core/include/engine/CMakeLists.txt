set(HEADER_CORE_ENGINE
    ${HEADER_CORE_ENGINE}
    ${CMAKE_CURRENT_SOURCE_DIR}/Neighbours.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Hamiltonian.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Hamiltonian_Anisotropic.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Hamiltonian_Isotropic.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Hamiltonian_Gaussian.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Optimizer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Optimizer_SIB.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Optimizer_SIB2.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Optimizer_Heun.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Optimizer_CG.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Optimizer_VP.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Method.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Method_LLG.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Method_GNEB.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Method_MMF.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Vectormath_Defines.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Vectormath.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Manifoldmath.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Managed_Allocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    PARENT_SCOPE
)