#!/bin/python3
"""
This is a notional strawman of what using Pymera for UQ might look like.
It uses only internal Femera models, deferring file format handling.
NOTE names starting with fmr: are reserved for internal Femera identifiers.
TODO change internal Femera identifiers to enums.
"""
import numpy as np

# Add path (parent directory) to find Pymera module.
import sys, os
sys.path.append(os.path.join(os.path.dirname(sys.path[0])))

import pymera
#import uqtools as uq

def main():
    """
    This performs a Monte Carlo simulation of a cantilever beam subjected to tip
    displacement. The simulation uses the Pymera library to set up and run the
    simulation, and collects the base stress for each simulation. The function
    takes no parameters and returns nothing.

    The simulation parameters are defined as follows: 
    - runs_n: the number of simulations to run (default: 1000) 
    - nominal_length: the length of the beam (nominal: 1.000) 
    - nominal_width: the width of the beam (nominal: 0.050) 
    - nominal_height: the height of the beam (nominal: 0.050) 
    - nominal_cell_size: the size of each cell in the beam mesh (nominal: 0.010)

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
    configured and installed.
    """
    #
    # Simulation nominal values -----------------------------------------------
    nominal_youngs = 210e9# Pa
    nominal_poissons = 0.285 #NOTE Clamp >= 0.25?
    #
    nominal_tip_z =-0.100# m
    #
    nominal_length = 1.000# m
    nominal_width = 0.050# m
    nominal_height = 0.050# m
    #
    nominal_dims = np.array([nominal_length, nominal_width, nominal_height])
    nominal_cell_size = 0.010# m
    #--------------------------------------------------------------------------
    # Structured discretization parameters.
    elem_count_xyz = np.array(nominal_dims / nominal_cell_size, dtype='u8')
    #
    fmr = pymera.Jobs()
    print('Hello ' + fmr.get_version() +' '+ fmr.get_name() + '!')
    #TODO Set Femera init options (MPI, OpenMP, verbosity, self-tests)?
    #TODO use python context:
    #     with pymera.Jobs() as fmr:
    #
    fmr.init()
    #**************************************************************************
    # Set nominal model parameters.
    #TODO Femera sims functions not implemented yet.
    #TODO changing internal femera fmr: string identifiers to enums.
    #NOTE fmr: string identifiers needed for JSON representation.
    #
    #TODO use a Runs object for run parameters?
    #
    #NOTE Nominal model setup could be done in a JSON file. ===================
    # sims.read('uq_straw_1c.json')
    #
    # Add simulation models.
    beam_sims = fmr.add_sims()#name='cantilever-beam-sims')#, runs_n=runs_n)
    #TODO use python context:
    #     with fmr.sims(name='cantilever-beam-sims') as beam_sims:
    # 
    #TODO Set model partitioning method.
    #
    #NOTE name is needed only for parameters and post-processing results.
    # Add model geometry.
    beam_geom = beam_sims.add_geometry(#name='beam-geometry',
                shape='fmr:geom:block')
    beam_geom.set(fmr.Data_type['Dimensions_xyz'],
                name='beam-dims', nominal=nominal_dims )
    #
    # Set material.
    beam_mtrl = beam_geom.set_material(#name='basic-steel',
                physics='fmr:mtrl:elastic:isotropic')
    beam_mtrl.set(fmr.Data_type['Youngs_modulus'],
                name='beam-youngs', nominal=nominal_youngs)
    beam_mtrl.set(fmr.Data_type['Poissons_ratio'],
                name='beam-poissons', nominal=nominal_poissons)
    #
    # Add mesh.
    beam_mesh = beam_geom.set_grid(#name='beam-mesh',
                #analysis_type='fmr:grid:FE',# optional?
                grid_structure='fmr:grid:structured',
                cell_type='fmr:cell_type:tet6',# 6 tets per cell
                elem_type='fmr:elem_type:tet10')# 10 nodes per tet elem.
    beam_mesh.set(fmr.Data_type['Grid_divs'], elem_count_xyz)
    # Set boundary conditions.
    beam_mesh.set_bcs(#name='fixed-base-bcs',
                nodes_at=fmr.Data_type['Node_x_min'],# Implied node set
                set=fmr.Data_type['Displacement_xyz'],
                to=0)# 'fmr:phys:bcs:encastre'
    beam_load = beam_mesh.add_bcs(
                nodes_at=fmr.Data_type['Node_x_max'],# Implied node set
                set=fmr.Data_type['Displacement_z'],
                name='tip-displace-bcs', nominal=nominal_tip_z)
    #
    # Set preconditioner and solver.
    beam_sims.set_preconditioner(method='fmr:solve:precon:jacobi')
    beam_sims.set_solver(method='fmr:solve:pcg')
    # defaults: analysis='fmr:solve:static', load_step_n=1, rtol=1e-6)
    #
    # Identify output parameters for post-processing.
    beam_results = beam_sims.add_post(name='base-force-mag',
                nodes_at=fmr.Data_type['Node_x_min'],# Implied node set
                sum=fmr.Data_type['Force_mag']# Returns 1 scalar for each sim
                )#, count=runs_n)#TODO try to hide this from the user.
    #==========================================================================
    #
    runs_n = 1000 # number of simulation runs
    # Set up random input variables as size runs_n numpy arrays.
    # (mean, stdev, N)
    length = np.random.normal(nominal_length, nominal_length/100, runs_n)
    width = np.random.normal(nominal_width, nominal_width/100, runs_n)
    height = np.random.normal(nominal_height, nominal_height/100, runs_n)
    tip_z = np.random.normal(nominal_tip_z, abs(nominal_tip_z)/10, runs_n)
    youngs = np.random.normal(nominal_youngs, nominal_youngs/10, runs_n)
    poissons = np.random.normal(nominal_poissons, nominal_poissons/10, runs_n)
    #
    # Set model parameters. These replace the nominal values.
    if True:
        fmr.set('beam-dims', [length, width, height]) # x,y,z
        fmr.set('tip-displace-bcs', tip_z)
        fmr.set('beam-youngs', youngs)# E
        fmr.set('beam-poissons', poissons)# nu
    else:#TODO alternative?
        beam_geom.set('beam-dims,', [length, width, height]) # x,y,z
        beam_load.set('tip-displace-bcs', tip_z)
        beam_mtrl.set('beam-youngs', youngs)# E
        beam_mtrl.set('beam-poissons', poissons)# nu
    #--------------------------------------------------------------------------
    #TODO Solve at nominal values for initial solution (u0) starting vector.
    #     Or, just keep the first solution and reuse it for u0.
    #NOTE Avoid relative tolerance (rtol) when providing a good initial guess.
    #
    beam_sims.init()# Optional: sims.run() will call sims.init() as needed.
    beam_sims.run()
    #
    # Get numpy array contents from Pymera.
    base_force = beam_results.get('base-force-mag')
    base_stress_avg = base_force / (width * height)# averaged over each base
    #
    beam_sims.exit() #NOTE invalidates sims post-processing pointers (base_force)
    #
    """
    #TODO UQ stuff, maybe create and run more sims,...
    # uq.do_some_stuff(base_stress_avg)
    """
    #**************************************************************************
    #
    print()
    print('base stress mean: ' + str(base_stress_avg.mean()))
    print('base stress standard deviation: ' + str(base_stress_avg.std()))
    print()
    fmr.exit()
    #
    #TODO more UQ stuff...
    #print(fmr.Data_type['Node_x_min'].value)

if __name__ == "__main__":
    main()