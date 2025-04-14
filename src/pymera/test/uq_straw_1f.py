#!/usr/bin/env python3
"""
This is a notional strawman of what using Pymera for UQ might look like.
It uses only internal Femera models, deferring file format handling.
NOTE names starting with fmr: are reserved for internal Femera identifiers.
"""
import numpy as np

import sys, os
sys.path.append(os.path.dirname(os.path.dirname(sys.path[0]))) # grandparent
sys.path.append(os.path.dirname(sys.path[0])) # parent

import pymera
from pymera.enumerators import fmr_enum, Data, Application, \
    Geometry_shape, Grid_structure, Part_method, Mtrl_physics, \
    Cell_type, Elem_type, Node_at
 #   Sims, Geom
    
#import uqtools as uq

from pymera_parse import full_path_json_file # temp. until put into pymera module

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
    if True: # Print full path JSON file.
        flat = full_path_json_file(json_filname)
        for key, value in flat.items():
            key_pretty = key.replace('\x1f', '\u00b7')
            print(f"{key_pretty}: {value}")
        print()
    #
    #TODO Set Femera init options (MPI, OpenMP, verbosity, self-tests)
    #     as arguments to the Jobs constructor.
    #--------------------------------------------------------------------------
    # Load simulations from JSON and CSV files.
    with pymera.Jobs() as jobs:
        print('\nHello '+jobs.get_version()+' '+jobs.get_name()+' (jobs)!')
        #
        # Read simulation set from JSON file.
        #sims_from_json = jobs.add_sims_from_file(json_filname)
        sims_from_json = jobs.add_sims_from_file(json_filname)
        #
        print('Hello '+sims_from_json.get_name()+' '+sims_from_json.get_version()+'!')
        #
        print('\nUser parameters, output fields, '
                + 'and internal variables in JSON file:')
        for name, value in jobs.parameter.items():
            if value is None:
                print(f'- {name}')
            else:
                print(f'- {name}: {value}')
        #
        # Read parameters and nominal values from the .csv file.
        jobs.add_parameter_file(csv_filname,
            has_names=True, has_nominals=csv_has_nominals)
        #
        if csv_has_nominals:
            print('\nUser parameter names (first row) '
                +'with nominal values (second row) from CSV file:')
            for name, nominal in jobs.nominal.items():
                print(f'- {name}: {nominal}')
        else:
            print('\nUser parameters (first row name)'
                +' added or updated from CSV file:')
            for name, value in jobs.parameter.items():
                print(f'- {name}: {value}')
        print('\nNumber of additional values (remaining rows) in the CSV file:')
        print(f'- sample_n: {jobs.parameter["sample_n"]}')
        print()
        #----------------------------------------------------------------------
        # Add beam_dimensions parameter [length, width, height] values.
        jobs.set_parameter('beam_dimensions',
            values=np.transpose(
                [jobs.parameter['length'],
                 jobs.parameter['width'],
                 jobs.parameter['height']]),
            nominal=
                [jobs.nominal['length'],
                 jobs.nominal['width'],
                 jobs.nominal['height']]
        )
        # sims.init # Initialize just this sims object.
        # jobs.sims_init() # Initializes all the sims.
        # jobs.run() is called on exit from the jobs "with" context block
        # and will initialize and then run each of the sims objects in turn.
        #
        # jobs.run() is called on exit from the jobs "with" context block.
        # * initializes (if needed) and runs all sims
        # * sims.init() also performs sanity checks. 
        #----------------------------------------------------------------------
        if False:
            # Set up the same sims manually.
            #
            #sims_py = jobs.add_sims(name='Cantilever beam sims (uq_straw_1.py)', 
            #    version='0.1.6', application=Application.UQ)
            sims_py = jobs.add_sims(name='Cantilever beam sims (uq_straw_1.py)', 
                version='0.1.6', application=Application.UQ)
            #
            #sims_py.set(Sims.NAME, "Fred")
            #sims_py.set(Sims.VERSION, "0.1.6")
            #sims_py.set(Application.UQ)
            #
            jobs.add_parameter("beam_dimensions",[1.0, 0.05, 0.05],
                                data=Data.DIMENSIONS_XYZ)
            jobs.add_parameter("tip_displace_z", -0.01, data=Data.DISPLACEMENT_Z)
            jobs.add_parameter("youngs", 210e9, data=Data.YOUNGS_MODULUS)
            jobs.add_parameter("poissons", 0.285, data=Data.POISSONS_RATIO)
            jobs.add_parameter("sample_n", 0, data=Data.RUNS_N)
            #
            jobs.add_parameter("node_number", Data.NODE_ID)
            jobs.add_parameter("nominal_coordinates", Data.NODE_XYZ)
            jobs.add_parameter("nominal_displacements", Data.DISPLACEMENT_XYZ)
            jobs.add_parameter("run_number", Data.RUNS_IX)
            jobs.add_parameter("beam_volume", Data.VOLUME)
            #
            #sims_py.add_parameter("beam-base-nodeset", Data.UNKNOWN)
            #sims_py.add_parameter("beam-tip-nodeset", Data.UNKNOWN)
            #sims_py.add_parameter("beam-force-mag", Data.UNKNOWN)
            #
            print(f'Hello {sims_py.get_name()} {sims_py.get_version()}!')
            print(f'Are you ready to do some {sims_py.get_application().name}?')
            #
            print('\nUser parameters, output fields, and internal variables:')
            for name, value in jobs.parameter.items():
                if value is None:
                    print(f'- {name}')
                else:
                    print(f'- {name}: {value}')
            #
            #TODO Implement geometry functions.
            geom = sims.add_geomety()
            geom.set(Geometry_shape.BLOCK)
            geom.set(Geom.DIMENSIONS, [1.0, 0.05, 0.05],
                              param='beam_dimensions')
            #
            grid = sims.add_grid(for_geometry=geom)
            grid.set(Grid_structure.RECTILINEAR)
            grid.set(Cell_type.BLOCK_6TET)
            grid.set(Elem_type.TET10)
            grid.set(Grid.GRID_DIVISIONS, [100, 5, 5])
            #
            grid.get(Node_at.NODE_X_MIN, param='beam-base-nodeset')
            grid.get(Node_at.NODE_X_MAX, param='beam-tip-nodeset')
            #
            grid.get(Grid.NODE_ID, param='node_number')
            grid.get(Grid.NODE_XYZ, param='nominal_coordinates')
            grid.get(Grid.VOLUME, param='beam_volume')
            #
            #part = sims.add_part(for_grid=grid)
            #part.set(Part_method.SINGLE)
            #
            bc_base = sims.add_bcs(name="Encastre the beam's base.")
            bc_base.set(BCs.NODE_SET, param='beam-base-nodeset')
            bc_base.set(BCs.DISPLACEMENT_XYZ, [0.0, 0.0, 0.0])
            #
            bc_tip = sims.add_bcs(name="Displace the beam's free end face.")
            bc_tip.set(BCs.NODE_SET, param='beam-tip-nodeset')
            bc_tip.set(BCs.DISPLACEMENT_Z,-0.010, param='tip_displace_z')
            #
            mtrl = sims.add_material(name="Basic steel")
            mtrl.set(Mtrl_physics.ELASTIC_ISOTROPIC)
            mtrl.set(Mtrl.YOUNGS_MODULUS, 210E9, param='youngs')
            mtrl.set(Mtrl.POISSONS_RATIO, 0.285, param='poissons')
            #
            solve = sims.add_solver(name='Solve nominal beam.')
            solve.set(Analysis.STATIC)
            solve.set(Conditioner_method.JACOBI)
            solve.set(Solve_method.PCG)
            solve.get(Solve.DISPLACEMENT_XYZ, param='nominal_displacements')
            #
            post = sims.add_post()
            post.set(Post.FILE_NAME, 'uq_straw_1_nominal.csv')
            post.set(Post.FIELD, param=['node_number',
                                        'nominal_coordinates',
                                        'nominal_displacements'])
            #
            runs = sims.add_runs()
            runs.set(Simulation_size.XS)
            runs.set(Part_method.SINGLE_PART)
            runs.set(Runs.CORES_PER_RUN, 1)
            runs.set(Runs.RUNS_N, 1)
            runs.set(Duplicate_runs.CPU_N)
            #
            runs.add_sims(0)
            runs.add_Geom(0)
            runs.add_Grid(0)
            runs.add_Part(0)
            runs.add_Mtrl(0)
            runs.add_BCs([0,1])
            runs.add_Solve(0)
            runs.add_Post(0)
            #
            #sims.init()
            jobs.run()
            #sims.exit() # invalidates sims post-processing pointers?
            #            # (e.g., base_force)
            #------------------------------------------------------------------
        print('Exiting jobs...')
    #--------------------------------------------------------------------------
    #
    # jobs.exit() is called before jobs is destroyed.
    #             * jobs is destroyed at the end of this function
    #             * exits (if needed) and deletes all sims first
    #----------------------------------------------------------------------
    # Get numpy array contents from Pymera.
    base_force = sims_from_json.get_post('base-force-mag')
    #
    base_stress_avg = base_force / (# averaged over each base area
        jobs.parameter['width'] * jobs.parameter['height'])
    print()
    print('base stress mean: ' + str(base_stress_avg.mean()))
    print('base stress standard deviation: ' + str(base_stress_avg.std()))
    print()
    #==========================================================================
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
    if False:# Write random input parameters to a .csv file.
        # Simulation nominal values -------------------------------------------
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
        # Structured discretization parameters --------------------------------
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