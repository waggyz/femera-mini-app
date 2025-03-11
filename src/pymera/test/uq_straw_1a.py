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
    beam_elem_dims = np.ascontiguousarray(
        np.array(nominal_dims / elem_size, dtype='u4'))
    
    # Set up random input variables as size N numpy arrays.
    tip_displacement_z = np.ascontiguousarray(#TODO hide ascontiguousarray.
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
    
    my_jobs = pym.jobs()
    sims = my_jobs.add_sims(name='cantilever-beam-sims', runs_n=N)
    
    sims.set_geometry(name='fmr:geom:block')# global geometry
    sims.set_bcs(name='fixed-base-bc',
                at='fmr:grid:node:x-min',
                set='fmr:phys:node:displacement:xyz',to='fmr:phys:bcs:encastre')
    sims.add_bcs(name='tip-z-displacement-bc',
                at='fmr:grid:node:x-max',
                set='fmr:phys:node:displacement:z', to='tip-displacement-z')
    
    sims.set_material(name='basic-steel',
                    physics='fmr:mtrl:linear-elastic-isotropic')
    
    sims.set_preconditioner(name='fmr:precon:jacobi')
    sims.set_solve(name='linear-solve', method='fmr:solve:PCG')
    # defaults: analysis='fmr:solve:static', load_step_n=1, rtol=1e-6)
    
    sims.set_grid(method='fmr:grid:FE', structured=true, elem='fmr:elem:tet10')#,
    #            elem_dims=beam_elem_dims)
    sims.set_partition_n(1)# 1 partition per model
    sims.set_parameter(name='fmr:grid:cell:dims', to=beam_elem_dims) #x
    sims.set_parameter('fmr:geom:block:width', width)# y
    sims.set_parameter(name='tip-displacement-z', to=tip_displacement_z)
    sims.set_parameter(name='fmr:geom:block:length', to=length) #x
    sims.set_parameter('fmr:geom:block:width', width)# y
    sims.set_parameter('fmr:geom:block:height', height)# z
    sims.set_parameter('fmr:mtrl:youngs-modulus', youngs)# E
    sims.set_parameter('fmr:mtrl:poissons-ratio', poissons)# nu
    
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