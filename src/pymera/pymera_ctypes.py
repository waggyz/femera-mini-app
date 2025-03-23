#!/usr/bin/env python3
import ctypes as ct

import os
libpath = os.path.join(os.getcwd(),'build','stage','i7-12800H','lib')

# Load the shared library
if os.name == 'posix':
    fmr = ct.CDLL(libpath + '/libfemerac.so')#TODO change fmr to jobs?
elif os.name == 'nt':
    # lib = ctypes.CDLL('./jobs.dll')
    raise OSError("Unsupported operating system")
else:
    raise OSError("Unsupported operating system")

# C/Python ctypes interface ===================================================
# Define the argument and return ctypes for C interface functions.

fmr_Dim_int = ct.c_ubyte
fmr_Exit_int = ct.c_int32
fmr_Enum_int = ct.c_int32
fmr_Local_int = ct.c_uint32
fmr_Global_int = ct.c_uint64

# Pass Jobs_t* as ct.c_void_p
fmr.fmr_new_jobs.restype = ct.c_void_p
fmr.fmr_delete_jobs.argtypes = [ct.c_void_p]

fmr.fmr_jobs_init.argtypes = [ct.c_void_p]
fmr.fmr_jobs_exit.argtypes = [ct.c_void_p]
fmr.fmr_jobs_did_init.restype = ct.c_bool
fmr.fmr_jobs_did_init.argtypes = [ct.c_void_p]

fmr.fmr_get_version.restype = ct.c_char_p
fmr.fmr_get_version.argtypes = [ct.c_void_p]
fmr.fmr_get_jobs_name.restype = ct.c_char_p
fmr.fmr_get_jobs_name.argtypes = [ct.c_void_p]

fmr.fmr_get_verbosity.restype = fmr_Dim_int
fmr.fmr_get_verbosity.argtypes = [ct.c_void_p]
fmr.fmr_set_verbosity.restype = fmr_Dim_int
fmr.fmr_set_verbosity.argtypes = [ct.c_void_p, fmr_Dim_int]

fmr.fmr_get_sims_n.restype = fmr_Local_int
fmr.fmr_get_sims_n.argtypes = [ct.c_void_p]
fmr.fmr_add_sims.argtypes = [ct.c_void_p]
fmr.fmr_add_sims.restype = fmr_Local_int

fmr.fmr_get_sims_name.restype = ct.c_char_p
fmr.fmr_get_sims_name.argtypes = [ct.c_void_p, fmr_Local_int]
fmr.fmr_set_sims_name.argtypes = [ct.c_void_p, fmr_Local_int, ct.c_char_p]

fmr.fmr_get_sims_version.restype = ct.c_char_p
fmr.fmr_get_sims_version.argtypes = [ct.c_void_p, fmr_Local_int]
fmr.fmr_set_sims_version.argtypes = [ct.c_void_p, fmr_Local_int, ct.c_char_p]