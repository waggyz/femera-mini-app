#!/bin/python3
"""
This is a notional strawman of what using Pymera for UQ might look like.
It uses only internal Femera models, deferring file format handling.
NOTE  names starting with fmr: are reserved for internal Femera identifiers.
"""
import pymera as pym
import uqtools as uq

N = 1000 # number of simulations

jobs = pym.jobs()
sims = jobs.add_sims(name='cantilever-beam-sims')
sims.set_runs_n(N)

sims.set_geometry(name='fmr:geom:block')# global geometry
sims.set_bcs(name='fixed-base-bc',
             at='fmr:geom:plane:x-min',
             set='fmr:phys:node:displacement:xyz', to='fmr:phys:bcs:encastre')
sims.add_bcs(name='tip-z-displacement-bc',
             at='fmr:geom:plane:x-max',
             set='fmr:phys:node:displacement:z', to='tip-displacement-z') 

sims.set_material(name='fmr:mtrl:steel',
                  physics='fmr:mtrl:linear-elastic-isotropic')

sims.set_preconditioner(name='fmr:precon:jacobi')
sims.set_solve(name='linear-solve', method='fmr:solve:PCG')
# defaults: analysis='fmr:solve:static', load_step_n=1, rtol=1e-6)

sims.set_grid(method='fmr:grid:FE', elem='fmr:elem:tet10', elem_size=0.020)
sims.set_partition_n(1)# 1 partition per model

# Set up random variables. These will all be size N arrays.
#TODO is 'mean' a synonym for 'to'?
sims.set_parameter(name='tip-displacement-z', mean=0.10, stdev=0.01)
sims.set_parameter(name='fmr:geom:length', to=1.000) #x (global parameter)
sims.set_parameter('fmr:geom:width',  0.050)# y
sims.set_parameter('fmr:geom:height', 0.050)# z
sims.set_parameter('fmr:mtrl:youngs-modulus', 210e9)# E
sims.set_parameter('fmr:mtrl:poissons-ratio', 0.30)# nu  

# Identify parameters for post-processing
sims.add_post('fmr:geom:parameters')# all geometry parameters set
sims.add_post('fmr:mtrl:parameters')# all material parameters set
sims.add_post('tip-displacement')
sims.add_post(name='base-force-mag',
              at='fmr:geom:plane:x-min',
              sum='fmr:phys:node:force:mag')

sims.init()
sims.run()

# post-process numpy arrays from Pymera
base_force = sims.get_post('base-force-mag')
base_area = ( sims.get_parameter('fmr:geom:width')
            * sims.get_parameter('fmr:geom:height') )
base_pressure_avg = base_force / base_area

sims.exit() #NOTE invalidates sims post-processing pointers

jobs.exit()
