#!/bin/python3
"""
This is a notional strawman of what using Pymera for UQ might look like.
It uses only internal Femera models, deferring file format handling.
NOTE names starting with fmr: are reserved for internal Femera identifiers.
"""
import pymera as fmr
import uqtools as uq
import numpy as np

def main():
    N = 1000 # number of simulations
    beam_length = 1.000
    beam_width = 0.050
    beam_height = 0.050
    elem_size = 0.010
    
    nominal_dims = np.array([beam_length, beam_width, beam_height])
    beam_elem_count = np.ascontiguousarray(
        np.array(nominal_dims / elem_size, dtype='u4'))
    
    # Set up random input variables as size N numpy arrays.
    tip_z = np.ascontiguousarray(#TODO hide ascontiguousarray.
        np.random.normal(0.100, 0.010, N)) # mean=0.100, stdev=0.010
    length = np.ascontiguousarray(
        np.random.normal(beam_length, beam_length/10, N))
    width = np.ascontiguousarray(
        np.random.normal(beam_width, beam_width/10, N))
    height = np.ascontiguousarray(
        np.random.normal(beam_height, beam_height/10, N))
    youngs = np.ascontiguousarray(
        np.random.normal(210e9, 210e8, N))
    poissons = np.ascontiguousarray(
        np.random.normal(0.285, 0.0285, N))
    
    my_jobs = fmr.jobs()
    sims = my_jobs.add_sims(name='cantilever-beam-sims', runs_n=N)
    
    sims.add_geometry(name='beam-geometry', shape='fmr:geom:block')
    sims.add_grid(name='beam-mesh', method='fmr:grid:FE', structured=true,
                 elem='fmr:elem:tet10')

    sims.set_bcs(name='fixed-base-bc',
                at='fmr:grid:node:x-min',
                set='fmr:phys:node:displacement:xyz',
                to='fmr:phys:bcs:encastre')
    sims.add_bcs(name='tip-bc',
                at='fmr:grid:node:x-max')# value is tip-bc parameter below
    
    sims.set_material(name='basic-steel',
                    physics='fmr:mtrl:linear-elastic-isotropic')
    
    sims.set_preconditioner(name='fmr:precon:jacobi')
    sims.set_solve(name='linear-solve', method='fmr:solve:PCG')
    # defaults: analysis='fmr:solve:static', load_step_n=1, rtol=1e-6)
    
    sims.set_partition_n(1)# one partition per model

    sims.set_parameter('beam-mesh','fmr:grid:cell_count_lwh', beam_elem_count)
    sims.set_parameter('tip-bc', 'fmr:phys:node:displacement:z', tip_z)
    sims.set_parameter('beam-geometry', 'fmr:geom:length', length) #x
    sims.set_parameter('beam-geometry', 'fmr:geom:width', width)# y
    sims.set_parameter('beam-geometry', 'fmr:geom:height', height)# z
    sims.set_parameter('basic-steel', 'fmr:mtrl:youngs-modulus', youngs)# E
    sims.set_parameter('basic-steel', 'fmr:mtrl:poissons-ratio', poissons)# nu
    
    # Identify parameters for post-processing.
    sims.add_post(name='base-force-mag',
                at='fmr:grid:node:x-min',
                sum='fmr:phys:node:force:mag')
    
    sims.init()
    sims.run()
    
    # Get numpy array contents from Pymera.
    base_force = sims.get_post('base-force-mag')
    base_pressure_avg = base_force / (width * height)
    
    sims.exit() #NOTE invalidates sims post-processing pointers
    
    my_jobs.exit()

if __name__ == "__main__":
    main()