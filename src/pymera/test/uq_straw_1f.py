#!/usr/bin/env python3
"""
This is a notional strawman of what using Pymera for UQ might look like.
It uses only internal Femera models, deferring file format handling.
NOTE names starting with fmr: are reserved for internal Femera identifiers.
TODO change internal Femera identifiers to enums.
"""
import numpy as np
import csv
import sys, os

# Add path (parent directory) to find Pymera module.
sys.path.append(os.path.join(os.path.dirname(sys.path[0])))

import pymera
#import uqtools as uq

from pymera_parse import flatten_json_file # temporary until integrated into pymera module

def main():
    """
    This performs a Monte Carlo simulation of a cantilever beam subjected to tip
    displacement. The simulation uses the Pymera library to set up and run the
    simulation, and collects the base stress for each simulation. The function
    takes no parameters and returns nothing.

    The simulation parameters are defined as follows: 
    - sample_n: the number of simulations to run (default: 1000) 
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
    here = os.path.dirname(os.path.abspath(__file__))
    #
    json_filname = os.path.join(here,'uq_straw_1f.json')
    csv_filname = os.path.join(here,'uq_straw_1f.csv')
    csv_has_nominals = True
    #
    if True: # Print flattened JSON file.
        flat = flatten_json_file(os.path.join(here, 'uq_straw_1f.json'))
        for key, value in flat.items():
            key_pretty = key.replace('\x1f', '\u00b7')
            print(f"{key_pretty} = {value}")
        print()
    #--------------------------------------------------------------------------
    if True: # Load simulations from JSON and CSV files.
        with pymera.Jobs() as jobs:
            print('\nHello '+jobs.get_version()+' '+jobs.get_name()+'!')
            #
            # Read simulation from JSON file.
            with jobs.add_sims_from_file(json_filname) as sims:
                print('Hello '+sims.get_name()+' '+sims.get_version()+'!')
                #
                print('\nUser parameters, output fields, '
                      + 'and internal parameters in JSON file:')
                for name, value in sims.parameter.items():
                    if value is not None:
                        print(f'- {name}: {value}')
                    else:
                        print(f'- {name}')
                #
                # Read parameters and nominal values from the .csv file.
                sims.add_parameter_file(csv_filname,
                    has_names=True, has_nominals=csv_has_nominals)
                #TODO Check if this is overwriting sims.parameter.
                #
                # Add dims parameter [length, width, height] values.
                v = [sims.parameter['length'],
                    sims.parameter['width'],
                    sims.parameter['height']]
                sims.add_parameter('dims',
                    nominal=[sims.nominal['length'],
                            sims.nominal['width'],
                            sims.nominal['height']],
                    values=list(map(list, zip(*v))) )# SoA to AoS
                print()
                # sims.init called on exit from sims with context block
            # jobs.run() called on exit from the jobs with context block
        # jobs.exit() called before jobs is destroyed.
    else:
        # Set up sims manually
        #jobs = pymera.Jobs()
        #print('Hello ' + jobs.get_version() +' '+ jobs.get_name() + '!')
        #TODO Set Femera init options (MPI, OpenMP, verbosity, self-tests)
        #     as arguments to the Jobs constructor.
        #jobs.init()
        #
        sims=jobs.add_sims()
        print('Hello ' +  sims.get_name() +' '+ sims.get_version() + '!')
        #
        # sims.init()# Optional: jobs.run() will call sims.init() as needed.
        #
        # jobs.run()
        #
        # sims.exit() #NOTE invalidates sims post-processing pointers
                    #     (e.g., base_force)
        #
        # jobs.exit()
        #
        pass
    #
    # Get numpy array contents from Pymera.
    base_force = sims.get_post('base-force-mag')
    #
    base_stress_avg = base_force / (# averaged over each base area
    sims.parameter['width'] * sims.parameter['height'])
    print()
    print('base stress mean: ' + str(base_stress_avg.mean()))
    print('base stress standard deviation: ' + str(base_stress_avg.std()))
    #--------------------------------------------------------------------------
    if csv_has_nominals:
        print('\nUser parameter names (first row) '
            +'with nominal values (second row) from CSV file:')
        for name, nominal in sims.nominal.items():
            print(f'- {name}: {nominal}')
    else:
        print('\nUser parameters (first row name)'
            +' added or updated from CSV file:')
        for name, value in sims.parameter.items():
            print(f'- {name}: {value}')
    print('\nNumber of additional values (remaining rows) in the CSV file:')
    print(f'- sample_n: {sims.sample_n}')
    print()
    '''
    # Nominal model setup  (same as the JSON file) ============================
    #
    sims = fmr.add_sims()
    # Set nominal model parameters.
    #TODO Femera sims functions not implemented yet.
    #
    '''
    """
    # Add model geometry.
    beam_geom = sims.add_geometry(#name='geometry',
                shape='fmr:geom:block')
    beam_geom.set(fmr.Data_type['Dimensions_xyz'],
                name='dims', nominal=nominal_dims )
    #
    # Set material.
    beam_mtrl = beam_geom.add_material(#name='basic-steel',
                physics='fmr:mtrl:elastic:isotropic')
    beam_mtrl.set(fmr.Data_type['Youngs_modulus'],
                name='youngs', nominal=nominal_youngs)
    beam_mtrl.set(fmr.Data_type['Poissons_ratio'],
                name='poissons', nominal=nominal_poissons)
    #
    # Add mesh.
    beam_mesh = beam_geom.add_grid(#name='mesh',
                #analysis_type='fmr:grid:FE',# optional?
                structured=True,
                cell_type='fmr:cell_type:tet6',# 6 tets per cell
                elem_type='fmr:elem_type:tet10')# 10 nodes per tet elem.
    beam_mesh.set(fmr.Data_type['Grid_divs'], elem_count_xyz)
    # Set boundary conditions.
    beam_mesh.add_bcs(#name='fixed-base-bcs',
                nodes_at=fmr.Data_type['Node_x_min'],# Implied node set
                set=fmr.Data_type['Displacement_xyz'],
                to=0.000)# 'fmr:phys:bcs:encastre'
    beam_load = beam_mesh.add_bcs(
                nodes_at=fmr.Data_type['Node_x_max'],# Implied node set
                set=fmr.Data_type['Displacement_z'],
                name='tip-displace-bcs', nominal=nominal_tip_z)
    #
    # Set preconditioner and solver.
    sims.set_preconditioner(method='fmr:solve:precon:jacobi')
    sims.set_solver(method='fmr:solve:pcg')
    # defaults: analysis='fmr:solve:static', load_step_n=1, rtol=1e-6)
    #
    # Identify output parameters for post-processing.
    beam_results = sims.add_post(name='base-force-mag',
                nodes_at=fmr.Data_type['Node_x_min'],# Implied node set
                sum=fmr.Data_type['Force_mag']# Returns 1 scalar for each sim
                )#, count=runs_n)
    """
    #==========================================================================
    if False:# Write random input parameters to a .csv file.
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
        # Structured discretization parameters. -------------------------------
        elem_count_xyz = np.array(nominal_dims / nominal_cell_size, dtype='u8')
        sample_n=100
        #----------------------------------------------------------------------
        #runs_n = sample_n+1 # includes a nominal sims run
        # Set up random input variables as size runs_n numpy arrays.
        # (mean, stdev, N)
        length = np.random.normal(nominal_length, nominal_length/100, sample_n)
        width = np.random.normal(nominal_width, nominal_width/100, sample_n)
        height = np.random.normal(nominal_height, nominal_height/100, sample_n)
        tip_z = np.random.normal(nominal_tip_z, abs(nominal_tip_z)/10, sample_n)
        youngs = np.random.normal(nominal_youngs, nominal_youngs/10, sample_n)
        poissons = np.random.normal(nominal_poissons, nominal_poissons/10, sample_n)
        #
        names=['length','width','height','tip_displace_z','youngs','poissons']
        nominals = [1.000,0.050,0.050, -0.010, 210e9,0.285]
        rows = zip(length, width, height, tip_z, youngs, poissons)
        with open('src/pymera/test/uq_straw_1f.csv', "w") as f:
            writer = csv.writer(f)
            writer.writerow(names)
            writer.writerow(nominals)
            for row in rows:
                writer.writerow(row)

if __name__ == "__main__":
    main()