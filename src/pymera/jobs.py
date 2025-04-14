#!/usr/bin/env python3
from pymera.enumerators import Data, Sims_file_format, Application
from pymera.pymera_libfemerac import libfemerac
from pymera_parse import parse_sims_from_file, full_path_json_file
from sims import Sims

import numpy as np
import csv
from warnings import warn
from atexit import register as atexit_register

class Jobs:
    def __init__(self): # create instance
        self.obj = libfemerac.fmr_new_jobs()
        self.nominal = {}# dictionary of nominal values
        self.parameter = {}# dictionary of parameters
        self.sims=[]
        self.sims_format_from_file_extension = {
            'json': Sims_file_format.JSON,
            'csv': Sims_file_format.CSV
        }
        atexit_register(self.exit_python)
    def __enter__(self): # enter context
        self.init()
        return(self)
    def __exit__(self, exception_type, exception_value, traceback):
        # leave context
        self.run()
    def __del__(self): # delete instance
        #self.exit()
        libfemerac.fmr_delete_jobs(self.obj)
    
    def exit_python(self):
        #print('exiting Python...')
        self.exit()

    #NOTE there is no method to remove Sims from Jobs.
    #     It would mess up the indexing.

    def add_parameter(self, name, values=None, data=None, nominal=None):
        """
        Adds a parameter to the Sims object.

        Parameters:
            name (str): The name of the parameter.

            nominal (Optional[Union[int, float, str, np.ndarray]]): The nominal
            value(s) of the parameter. If a single value is provided, it will be
            stored as a scalar. If multiple values are provided, they will be
            stored as a numpy contiguous array. Defaults to None.
            
            values (Optional[Union[int, float, str, np.ndarray]]): The values of
            the parameter. If a single value is provided, it will be stored as a
            scalar. If multiple values are provided, they will be stored as a
            numpy contiguous array. Defaults to None.

        Returns:
            None
        """
        if isinstance(nominal, (list, tuple, set)):
            if len(nominal) > 1:
                nominal = np.ascontiguousarray(nominal)
        self.nominal[name] = nominal
        if isinstance(values, (list, tuple, set)):
            if len(values) > 1:
                values = np.ascontiguousarray(values)
        self.parameter[name] = values

    def set_parameter(self, name, values=None, data=None, nominal=None):#TODO same as add_?
        if isinstance(nominal, (list, tuple, set)):
            if len(nominal) > 1:
                nominal = np.ascontiguousarray(nominal)
        self.nominal[name] = nominal
        if isinstance(values, (list, tuple, set)):
            if len(values) > 1:
                values = np.ascontiguousarray(values)
        self.parameter[name] = values
    #def set_parameter(self, component, parameter, value):
    #    if component not in self.parameters:
    #        self.parameters[component] = {}
    #    self.parameters[component][parameter] = value
    
    def get_parameter(self, component, parameter):
        return self.parameters[component][parameter]

    def add_parameter_file(self, filename, has_names=True, has_nominals=False):
        """
        Adds parameters from a CSV file to the parameter dictionary and the
        nominal dictionary. The function takes the following arguments:
        
        - filename (str): The path to the CSV file containing the parameters.
        - has_names (bool, optional): Whether the first line of the CSV file
          contains parameter names. Defaults to True.
        - has_nominals (bool, optional): Whether the next line of the CSV file
          contains nominal values. Defaults to False.
        
        The function reads the CSV file and populates the parameter and nominal
        dictionaries with the values. If has_names is True, the function assumes
        that the first line of the CSV file contains the parameter names and
        creates an empty list for each parameter. If has_nominals is True, the
        function assumes that the second line of the CSV file contains the
        nominal values and sets each nominal value to None. The function then
        appends each value from the remaining lines of the CSV file to the
        corresponding parameter list. If the number of values for a parameter is
        greater than 1, the function converts the list to a contiguous numpy
        array.
        
        The function sets the parameter "sample_n" to the minimum number of
        values of all parameters.
        """
        with open(filename, 'r') as f:
            reader = csv.reader(f)
            if has_names: # first line has parameter names
                names = next(reader)
            else: # Make placeholder names: param_1, param_2,, param_3,...
                line = next(reader)# need a line to get length
                names =  [f'param_{i+1}' for i in range(len(line))]
                f.seek(0)# Return to top.
            # Make dictionaries having keys for names and empty arrays (for
            # parameter) or None (for nominal) for values.
            for name in names:
                self.parameter[name] = []
            self.nominal = {name: None for name in names}
            # The next (second) line has the nominal values (if provided)
            if has_nominals:
                nominals = next(reader)
                for name, val in zip(names, nominals):
                    self.nominal[name] = float(val)#TODO Check type of each val?
            for row_col in reader:
                for name, col in zip(names, row_col):
                    self.parameter[name].append(float(col))
            self.parameter["sample_n"] = min(len(self.parameter[name])
                                              for name in names)
            if(self.parameter["sample_n"] > 1):
                # Convert lists to contiguous numpy arrays
                for key in self.parameter:
                    self.parameter[key] = np.ascontiguousarray(
                        self.parameter[key])
            if(self.parameter["sample_n"] != max(len(self.parameter[name])
                                                for name in names)):
                warn('\n pym Jobs WARN Not all parameters have the same number '
                     +'of values. '
                     +'Something might be wrong with your CSV file.')

    def get_sims_n(self):
        return libfemerac.fmr_get_sims_n(self.obj)
    
    def add_sims(self, name=None, version=None,
                  application=Application.UNKNOWN):
        self.sims.append(Sims(self, name=name, version=version,
                               application=application))
        return self.sims[-1]

    def add_sims_from_file(self, filename, format=Sims_file_format.UNKNOWN):
        if(format == Sims_file_format.UNKNOWN):
            if '.' in filename: # Infer format from file extension.
                ext = filename.split('.')[-1]
                format = self.sims_format_from_file_extension[ext]
        if(format == Sims_file_format.UNKNOWN):
            print(f'WARNING Unknown sims file format: {filename}') # throw?
        if((format.value > Sims_file_format.UNKNOWN.value) and
           (format != Sims_file_format.END)):
            self.sims.append(parse_sims_from_file(Sims(self), filename, format))
            return self.sims[-1]

    def init(self):
        libfemerac.fmr_jobs_init(self.obj)
    
    def run(self):
        #fmr.fmr_jobs_run(self.obj)
        print('Running '+str(self.get_sims_n())+' simulations.... (jk ;)')
        pass

    def exit(self):
        if libfemerac.fmr_jobs_did_init(self.obj):
            libfemerac.fmr_jobs_exit(self.obj)

    def did_init(self):
        return libfemerac.fmr_jobs_did_init(self.obj)
    
    def get_version(self):
        return libfemerac.fmr_get_version(self.obj).decode('utf-8', errors='replace')
    
    def get_name(self):
        return libfemerac.fmr_get_jobs_name(self.obj).decode('utf-8', errors='replace')
    
    def get_verbosity(self):
        return libfemerac.fmr_get_verbosity(self.obj)

    def set_verbosity(self, verbosity=3):
        # Clamp values to range of ct.c_ubyte.
        if(verbosity<0):
            verbosity=0
        elif(verbosity>255):
            verbosity=255
        return libfemerac.fmr_set_verbosity(self.obj, verbosity)
    
