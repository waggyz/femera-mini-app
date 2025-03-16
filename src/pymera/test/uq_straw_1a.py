#!/bin/python3
"""
This is a notional strawman of what using Pymera for UQ might look like.
It uses only internal Femera models, deferring file format handling.
runs_nOTE names starting with fmr: are reserved for internal Femera identifiers.
"""
# Add path (parent directory) to find Pymera module.
import sys, os
sys.path.append(os.path.join(os.path.dirname(sys.path[0])))

import pymera as fmr
#import uqtools as uq

import numpy as np

def main():
    """
    This performs a Monte Carlo simulation of a cantilever beam subjected to tip
    displacement. The simulation uses the Pymera library to set up and run the
    simulation, and collects the base stress for each simulation. The function
    takes no parameters and returns nothing.

    The simulation parameters are defined as follows: 
    - runs_n: the number of simulations to run (default: 1000) 
    - beam_length: the length of the beam (default: 1.000) 
    - beam_width: the width of the beam (default: 0.050) 
    - beam_height: the height of the beam (default: 0.050) 
    - cell_size: the size of each cell in the beam mesh (default: 0.010)

    The function initializes the Pymera Jobs object, sets the simulation
    options, adds the simulation models, sets the model partitioning method,
    adds the model geometry and mesh, sets the boundary conditions, sets the
    material properties, sets the preconditioner and solver, identifies the
    output parameters, initializes the simulation, runs the simulation, and
    collects the base stress values.

    After the simulation is complete, the function calculates the mean and
    standard deviation of the base stress values, and prints them to the
    console.

    Note: The function assumes that the Pymera library is properly
    installed and configured.
    """
    runs_n = 1000 # number of simulation runs
    #
    youngs_modulus = 210e9
    poissons_ratio = 0.285
    #
    tip_displacement =-0.100
    #
    beam_length = 1.000
    beam_width = 0.050
    beam_height = 0.050
    #
    cell_size = 0.010
    #--------------------------------------------------------------------------
    nominal_dims = np.array([beam_length, beam_width, beam_height])
    beam_elem_count = np.array(nominal_dims / cell_size, dtype='u8')
    #
    # Set up random input variables as size runs_n numpy arrays.
    # (mean, stdev, N)
    tip_z = np.random.normal(tip_displacement, abs(tip_displacement)/10, runs_n)
    length = np.random.normal(beam_length, beam_length/100, runs_n)
    width = np.random.normal(beam_width, beam_width/100, runs_n)
    height = np.random.normal(beam_height, beam_height/100, runs_n)
    youngs = np.random.normal(youngs_modulus, youngs_modulus/10, runs_n)
    poissons = np.random.normal(poissons_ratio, poissons_ratio/10, runs_n)
    #
    fmr_jobs = fmr.Jobs()
    print('Hello ' + fmr_jobs.get_version() + '!')
    #TODO Set Femera init options?
    #--------------------------------------------------------------------------
    fmr_jobs.init()
    #
    sims = fmr_jobs.add_sims(name='cantilever-beam-sims', runs_n=runs_n)
    #
    # Set input parameters for each run.
    #TODO use a Runs object for run parameters?
    sims.set_parameter('beam-mesh', 'fmr:grid:cell_count_lwh', beam_elem_count)
    #
    sims.set_parameter('tip-displace-bc', 'fmr:phys:node:displacement:z', tip_z)
    sims.set_parameter('beam-geometry', 'fmr:geom:length', length) #x
    sims.set_parameter('beam-geometry', 'fmr:geom:width', width)# y
    sims.set_parameter('beam-geometry', 'fmr:geom:height', height)# z
    sims.set_parameter('basic-steel', 'fmr:mtrl:youngs-modulus', youngs)# E
    sims.set_parameter('basic-steel', 'fmr:mtrl:poissons-ratio', poissons)# nu
    #
    #NOTE Model setup could be done in a JSON file. ===========================
    # sims.read('uq_straw_1a.json')
    # These are the same for every run.
    # 
    # Set model partitioning method.
    sims.set_partition_n(1)# one partition per model
    #
    # Add model geometry and mesh.
    sims.add_geometry(name='beam-geometry', shape='fmr:geom:block')
    sims.add_grid(name='beam-mesh',
                  for_geometry='beam-geometry',# optional, last geometry assumed
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
    sims.set_preconditioner(method='fmr:solve:precon:jacobi')
    sims.set_solver(name='linear-solve', method='fmr:solve:pcg')
    # defaults: analysis='fmr:solve:static', load_step_n=1, rtol=1e-6)
    #
    # Identify output parameters for post-processing.
    sims.add_post(name='base-force-mag',
                  at='fmr:grid:node:x-min',
                  sum='fmr:phys:node:force:mag')
    #==========================================================================
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
    print('base stress mean: ' + str(base_stress_avg.mean()))
    print('base stress standard deviation: ' + str(base_stress_avg.std()))
    """
    #TODO UQ stuff, maybe create and run more sims,...
    uq.do_some_stuff(base_stress_avg)
    """
    print('DONE')
    fmr_jobs.exit()
    #
    #TODO more UQ stuff...

if __name__ == "__main__":
    main()