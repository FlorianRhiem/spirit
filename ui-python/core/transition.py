import core.corelib as corelib
import ctypes

### Load Library
_core = corelib.LoadCoreLibrary()


### Generate homogeneous transition between two images of a chain
_Homogeneous             = _core.Transition_Homogeneous
_Homogeneous.argtypes    = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_int]
_Homogeneous.restype     = None
def Homogeneous(p_state, idx_1, idx_2, idx_chain=-1):
    _Homogeneous(p_state, idx_1, idx_2, idx_chain)

### Add some temperature-scaled noise to a transition between two images of a chain
_Add_Noise_Temperature             = _core.Transition_Add_Noise_Temperature
_Add_Noise_Temperature.argtypes    = [ctypes.c_void_p, ctypes.c_float, ctypes.c_int, ctypes.c_int, ctypes.c_int]
_Add_Noise_Temperature.restype     = None
def Add_Noise_Temperature(p_state, temperature, idx_1, idx_2, idx_chain=-1):
    _Add_Noise_Temperature(p_state, temperature, idx_1, idx_2, idx_chain)