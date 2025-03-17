#!/bin/python3
import numpy as np
import ctypes as ct
import csv
from enum import Enum

import os
libpath = os.path.join(os.getcwd(),'build','stage','i7-12800H','lib')

# Load the shared library
if os.name == 'posix':
    fmr = ct.CDLL(libpath + '/libfemerac.so')
elif os.name == 'nt':
    # lib = ctypes.CDLL('./jobs.dll')
    raise OSError("Unsupported operating system")
else:
    raise OSError("Unsupported operating system")

# Enums =======================================================================

def create_enum_from_csv(file_path, enum_name, start_at=None):
    # Read the CSV file
    # from NASA ChatGSFC, Claude 3.5 Sonnet v2
    with open(file_path, 'r') as csv_file:
        reader = csv.reader(csv_file)
        # Assuming the first row contains headers, skip it
        next(reader, None)
        
        # Create a dictionary to hold enum members
        enum_members = {}
        if(start_at == None):
            # Iterate through the rows and add them to the enum_members dictionary
            for row in reader:
                if len(row) >= 2:  # Ensure we have at least a name and a value
                    name, value = row[0], row[1]
                    enum_members[name] = value
        else:
            row_i = start_at
            for row in reader:
                if len(row) >= 1:  # Ensure we have at least a name
                    name = row[0]
                    enum_members[name] = row_i
                    row_i += 1
    #
    # Create and return the Enum class dynamically
    return Enum(enum_name, enum_members)

#==============================================================================
#TODO Need specifiers for
#    fmr::Exit_int, fmr::Dim_int, fmr::Enum_int, fmr::Local_int, fmr::Global_int
#    ...
# Define the argument and return types for C interface functions.
fmr.new_jobs.restype = ct.c_void_p
fmr.delete_jobs.argtypes = [ct.c_void_p]
fmr.jobs_init.argtypes = [ct.c_void_p]
fmr.jobs_exit.argtypes = [ct.c_void_p]
fmr.jobs_did_init.restype = ct.c_bool
fmr.jobs_did_init.argtypes = [ct.c_void_p]
fmr.get_version.restype = ct.c_char_p
fmr.get_version.argtypes = [ct.c_void_p]
fmr.jobs_get_name.restype = ct.c_char_p
fmr.jobs_get_name.argtypes = [ct.c_void_p]
fmr.get_verbosity.restype = ct.c_ubyte
fmr.get_verbosity.argtypes = [ct.c_void_p]
fmr.set_verbosity.restype = ct.c_ubyte
fmr.set_verbosity.argtypes = [ct.c_void_p, ct.c_ubyte]

Data_type = create_enum_from_csv(
    os.path.join(os.getcwd(),'data','src','data-type.csv'),
    'Data_type', start_at=3)

# Create a class to represent the Jobs object in Python.
class Jobs:
    def __init__(self, name='fmr:user:jobs'):
        self.obj = fmr.new_jobs()
        self.name = name
        # enum

    def __del__(self):
        if fmr.jobs_did_init(self.obj):
            fmr.jobs_exit(self.obj)#TODO automatically exit here?
        fmr.delete_jobs(self.obj)
    
    def add_sims(self, name='fmr:user:sims', runs_n=1):
        return Sims(self, name=name, runs_n=runs_n)

    def init(self):
        fmr.jobs_init(self.obj)

    def exit(self):
        fmr.jobs_exit(self.obj)

    def did_init(self):
        return fmr.jobs_did_init(self.obj)
    
    def get_version(self):
        return fmr.get_version(self.obj).decode('utf-8', errors='replace')
    
    def get_name(self):
        return fmr.jobs_get_name(self.obj).decode('utf-8', errors='replace')
    
    def get_verbosity(self):
        return fmr.get_verbosity(self.obj)
    def set_verbosity(self, verbosity=3):
        # Clamp values to range of ct.c_ubyte.
        if(verbosity<0):
            verbosity=0
        elif(verbosity>255):
            verbosity=255
        return fmr.set_verbosity(self.obj, verbosity)

# Create a class to represent the Sims object in Python.
class Sims:
    def __init__(self, jobs, name='fmr:user:sims', runs_n=1):
        """
        Initializes a Sims object with the given parameters.

        Parameters:
            jobs (Jobs_t): The Jobs object to associate with the Sims object.
            name (str, optional): The name of the Sims object. Defaults to
                'fmr:user:sims'.
            runs_n (int, optional): The number of runs for the Sims object.
                Defaults to 1.

        Attributes:
            jobs (Jobs_t): The Jobs object associated with the Sims object.
            name (str): The name of the Sims object.
            runs_n (int): The number of runs for the Sims object.
            geometries (list): The list of geometries associated with the Sims
                object.
            grids (list): The list of grids associated with the Sims object.
            bcs (list): The list of boundary conditions associated with the Sims
                object.
            materials (list): The list of materials associated with the Sims
                object.
            preconditioner (object or None): The preconditioner associated with
                the Sims object. Defaults to None.
            solve (object or None): The solver associated with the Sims object.
                Defaults to None.
            post_processes (list): The list of post-processing functions
                associated with the Sims object.
            parameters (dict): The parameters associated with the Sims object.
            partition_n (int): The partition number associated with the Sims
                object. Defaults to 1.
        """
        self.jobs = jobs
        self.name = name
        self.runs_n = runs_n
        self.geometries = []
        self.grids = []
        self.bcs = []
        self.materials = []
        self.preconditioner = None
        self.solve = None
        self.post_processes = []
        self.parameters = {}
        self.partition_n = 1

    def set_partition_n(self, n=1):
        self.partition_n = n

    def add_geometry(self, name, shape):
        self.geometries.append({"name": name, "shape": shape})

    def add_grid(self, name, for_geometry=None, type=None, method=None, elem=None):
        if for_geometry is None and self.geometries:
            for_geometry = self.geometries[-1]["name"]
        self.grids.append({
            "name": name,
            "for": for_geometry,
            "type": type,
            "method": method,
            "elem": elem
        })

    def set_bcs(self, name, at, set, to):
        self.bcs = []
        self.bcs.append({
            "name": name,
            "at": at,
            "set": set,
            "to": to
        })

    def add_bcs(self, name, at):
        self.bcs.append({
            "name": name,
            "at": at
        })

    def set_material(self, name, physics):
        self.materials.append({
            "name": name,
            "physics": physics
        })

    def set_preconditioner(self, name=None, method=None):
        self.preconditioner = {
            "name" : name,
            "method": method
        }

    def set_solver(self, name, method,
                   analysis='fmr:solve:static', load_step_n=1, rtol=1e-6):
        self.solve = {
            "name": name,
            "method": method,
            "analysis": analysis,
            "load_step_n": load_step_n,
            "rtol": rtol
        }

    def add_post(self, name, at, sum, count=-1):
        #if(count<0):
        #    self.count=self.runs_n
        self.post_processes.append({
            "name": name,
            "at": at,
            "sum": sum,
            "count": count
        })

    def get_post(self, name):
        # Placeholder for getting post-processing results
        # In a real implementation, this would return actual data.
        return np.ones(self.runs_n)

    def set_parameter(self, component, parameter, value):
        if component not in self.parameters:
            self.parameters[component] = {}
        self.parameters[component][parameter] = value
    
    def get_parameter(self, component, parameter):
        return self.parameters[component][parameter]

    def init(self):
        # Placeholder for initialization logic
        pass

    def run(self):
        # Placeholder for run logic
        pass

    def exit(self):
        # Placeholder for cleanup logic
        pass


