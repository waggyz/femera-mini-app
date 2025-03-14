#!/bin/python3
"""
This is a notional strawman of what using Pymera for UQ might look like.
It uses only internal Femera models, deferring file format handling.
NOTE names starting with fmr: are reserved for internal Femera identifiers.
"""
# Add path (parent directory) to find Pymera module.
import sys, os
sys.path.append(os.path.join(os.path.dirname(sys.path[0])))

import pymera as fmr
#import uqtools as uq

import numpy as np

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
    length = np.random.normal(beam_length, beam_length/100, N)
    width = np.random.normal(beam_width, beam_width/100, N)
    height = np.random.normal(beam_height, beam_height/100, N)
    youngs = np.random.normal(210e9, 210e8, N)
    poissons = np.random.normal(0.285, 0.0285, N)
    #
    fmr_jobs = fmr.Jobs()
    print ('Hello '+ fmr_jobs.get_version() + '!')
    #TODO Set Femera init options?
    fmr_jobs.init()
    #
    sims = fmr_jobs.add_sims(name='cantilever-beam-sims', runs_n=N)
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
                  for_geometry='beam-geometry',# optional, assumes last geometry added
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
    # Set input parameters.
    sims.set_parameter('beam-mesh','fmr:grid:cell_count_lwh', beam_elem_count)
    sims.set_parameter('tip-displace-bc', 'fmr:phys:node:displacement:z', tip_z)
    sims.set_parameter('beam-geometry', 'fmr:geom:length', length) #x
    sims.set_parameter('beam-geometry', 'fmr:geom:width', width)# y
    sims.set_parameter('beam-geometry', 'fmr:geom:height', height)# z
    sims.set_parameter('basic-steel', 'fmr:mtrl:youngs-modulus', youngs)# E
    sims.set_parameter('basic-steel', 'fmr:mtrl:poissons-ratio', poissons)# nu
    #
    sims.init()# Optional: sims.run() will call sims.init() as needed.
    sims.run()
    #
    # Get numpy array contents from Pymera.
    base_force = sims.get_post('base-force-mag')
    base_stress_avg = base_force / (width * height)# averaged over each base
    #
    sims.exit() #NOTE invalidates sims post-processing pointers (base_force)
    #
    print ('base stress mean: ' + str(base_stress_avg.sum() / N) )
    print ('base stress standard deviation: ' + str(base_stress_avg.std()) )
    """
    #TODO UQ stuff, maybe create and run more sims,...
    uq.do_some_stuff(base_stress_avg)
    """
    print ('DONE')
    fmr_jobs.exit()

if __name__ == "__main__":
    main()