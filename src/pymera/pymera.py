#!/bin/python3
import numpy as np
import ctypes as ct

import os, sys
basedir = os.path.dirname(os.path.dirname(os.path.dirname(sys.path[0])))
libpath = os.path.join(basedir,'build','stage','i7-12800H','lib')

# Load the shared library
if os.name == 'posix':
    fmr = ct.CDLL(libpath + '/libfemerac.so')
elif os.name == 'nt':
    #lib = ctypes.CDLL('./jobs.dll')
    raise OSError("Unsupported operating system")
else:
    raise OSError("Unsupported operating system")

# Define the argument and return types for C interface functions.
fmr.new_jobs.restype = ct.c_void_p
fmr.delete_jobs.argtypes = [ct.c_void_p]
fmr.jobs_init.argtypes = [ct.c_void_p]
fmr.jobs_exit.argtypes = [ct.c_void_p]
fmr.get_version.restype = ct.c_char_p
fmr.get_version.argtypes = [ct.c_void_p]

# Create a class to represent the Sims object in Python.
class Sims:
    def __init__(self, jobs, name='fmr:user:sims', runs_n=0):
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

    def set_preconditioner(self, name):
        self.preconditioner = name

    def set_solve(self, name, method, analysis='fmr:solve:static', load_step_n=1, rtol=1e-6):
        self.solve = {
            "name": name,
            "method": method,
            "analysis": analysis,
            "load_step_n": load_step_n,
            "rtol": rtol
        }

    def add_post(self, name, at, sum):
        self.post_processes.append({
            "name": name,
            "at": at,
            "sum": sum
        })

    def get_post(self, name):
        # Placeholder for getting post-processing results
        # In a real implementation, this would return actual data
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

# Create a class to represent the Jobs object in Python.
class Jobs:
    def __init__(self, name='fmr:user:jobs'):
        self.obj = fmr.new_jobs()
        self.name = name

    def __del__(self):
        fmr.delete_jobs(self.obj)
    
    def add_sims(self, name='fmr:user:sims', runs_n=1):
        return Sims(self, name=name, runs_n=runs_n)

    def init(self):
        fmr.jobs_init(self.obj)

    def exit(self):
        fmr.jobs_exit(self.obj)
    
    def get_version (self):
        return fmr.get_version(self.obj).decode('utf-8', errors='replace')
