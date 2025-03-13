#!/bin/python3
"""
This is a notional strawman of what using Pymera for UQ might look like.
It uses only internal Femera models, deferring file format handling.
NOTE names starting with fmr: are reserved for internal Femera identifiers.
"""
#import pymera as fmr
#import uqtools as uq
import numpy as np

import ctypes as ct
import os

libpath='/home/dwagner5/Code/femera-mini-cmake/build/stage/i7-12800H/lib/'

# Load the shared library
if os.name == 'posix':
    lib = ct.CDLL(libpath + 'libfemerac.so')
elif os.name == 'nt':
    #lib = ctypes.CDLL('./jobs.dll')
    raise OSError("Unsupported operating system")
else:
    raise OSError("Unsupported operating system")

# Define the argument and return types for the C interface functions
lib.newc_jobs.restype = ct.c_void_p
lib.delete_jobs.argtypes = [ct.c_void_p]
lib.jobs_init.argtypes = [ct.c_void_p]
lib.jobs_exit.argtypes = [ct.c_void_p]

# Create a class to represent the Jobs object in Python
class Jobs:
    def __init__(self):
        self.obj = lib.newc_jobs()

    def init(self):
        lib.jobs_init(self.obj)

    def exit(self):
        lib.jobs_exit(self.obj)

    def __del__(self):
        lib.delete_jobs(self.obj)

def main():
    N = 1000 # number of simulations
    beam_length = 1.000
    beam_width = 0.050
    beam_height = 0.050
    cell_size = 0.010
    #
    nominal_dims = np.array([beam_length, beam_width, beam_height])
    beam_elem_count = np.array(nominal_dims / cell_size, dtype='u4')
    #
    # Set up random input variables as size N numpy arrays.
    tip_z = np.random.normal(0.100, 0.010, N) # mean=0.100, stdev=0.010
    length = np.random.normal(beam_length, beam_length/10, N)
    width =np.random.normal(beam_width, beam_width/10, N)
    height = np.random.normal(beam_height, beam_height/10, N)
    youngs = np.random.normal(210e9, 210e8, N)
    poissons = np.random.normal(0.285, 0.0285, N)
    #
    my_jobs = Jobs()
    my_jobs.init()
    #
    """
    sims = my_jobs.add_sims(name='cantilever-beam-sims', runs_n=N)
    #
    #NOTE Model setup could be done in a JSON file. ---------------------------
    # sims.read('uq_straw_1a.json')
    # 
    # Set model partitioning method.
    sims.set_partition_n(1)# one partition per model
    #
    # Add model geometry and mesh.
    sims.add_geometry(name='beam-geometry', shape='fmr:geom:block')
    sims.add_grid(name='beam-mesh',
                  for='beam-geometry',# optional, assumes last geometry added
                  type='fmr:grid:FE',
                  method='fmr:grid:structured',
                  elem='fmr:elem:tet10')
    #
    # Set boundary conditions.
    sims.set_bcs(name='fixed-base-bc',
                 at='fmr:grid:node:x-min',
                 set='fmr:phys:node:displacement:xyz',
                 to='fmr:phys:bcs:encastre')
    sims.add_bcs(name='tip-displace-bc',
                 at='fmr:grid:node:x-max')# value is tip-bc parameter below
    #
    # Set material.
    sims.set_material(name='basic-steel',
                      physics='fmr:mtrl:linear-elastic-isotropic')
    #
    # Set preconditioner and solver.
    sims.set_preconditioner(name='fmr:solve:precon:jacobi')
    sims.set_solve(name='linear-solve', method='fmr:solve:PCG')
    # defaults: analysis='fmr:solve:static', load_step_n=1, rtol=1e-6)
    #
    # Identify output parameters for post-processing.
    sims.add_post(name='base-force-mag',
                at='fmr:grid:node:x-min',
                sum='fmr:phys:node:force:mag')
    #--------------------------------------------------------------------------
    #
    # Set input parameters.
    sims.set_parameter('beam-mesh','fmr:grid:cell_count_lwh', beam_elem_count)
    sims.set_parameter('tip-displace-bc', 'fmr:phys:node:displacement:z', tip_z)
    sims.set_parameter('beam-geometry', 'fmr:geom:length', length) #x
    sims.set_parameter('beam-geometry', 'fmr:geom:width', width)# y
    sims.set_parameter('beam-geometry', 'fmr:geom:height', height)# z
    sims.set_parameter('basic-steel', 'fmr:mtrl:youngs-modulus', youngs)# E
    sims.set_parameter('basic-steel', 'fmr:mtrl:poissons-ratio', poissons)# nu
    #
    sims.init()# optional, sims.run() will call sims.init()
    sims.run()
    #
    # Get numpy array contents from Pymera.
    base_force = sims.get_post('base-force-mag')
    base_stress_avg = base_force / (width * height)
    #
    sims.exit() #NOTE invalidates sims post-processing pointers (base_force)
    #
    #TODO UQ stuff, maybe create and run more sims,...
    uq.do_some_stuff(base_stress_avg)
    #"
    """
    my_jobs.exit()

if __name__ == "__main__":
    main()