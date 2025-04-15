#!/usr/bin/env python3
from pymera.enumerators import fmr_enum, Data, Grid_structure, \
    Sims_file_format

import numpy as np
import os, json, csv

def find_keys_used(obj, user_keys=True, enum_keys=False):
    """
    Recursively traverses a nested dictionary or list and returns a dictionary
    containing the keys that are not prefixed with "fmr:" and their
    corresponding values.

    NOTE Currently, this function also extracts fmr: enums.
    
    Parameters:
        obj (dict or list): The object to traverse.
        
    Returns:
        dict: A dictionary containing the keys that are not prefixed with "fmr:"
        and their corresponding values.
    """
    result = {}
    if isinstance(obj, dict):
        for key, val in obj.items():
            if enum_keys and (key in fmr_enum):
                result[key] = fmr_enum[key][val.split('fmr:')[1].upper()]
                #result[key] = fmr_enum[key][val.upper()].value # integer
            if not key.startswith("fmr:"):
                if isinstance(val, (int, float, str)):
                    result[key] = val
                elif isinstance(val, (dict,list)):
                    #TODO resolve datatype of value key.
                    nominal = val#next(iter(val.values()))
                    if isinstance(nominal, (list, tuple, set)):
                        if len(nominal) > 1:
                            nominal = np.ascontiguousarray(nominal)
                    elif not isinstance(nominal, (int, float, str)):
                        nominal = None #str(next(iter(val.keys())))# datatype
                    result[key] = nominal
                if isinstance(val, (dict, list)):
                    result.update(find_keys_used(val, user_keys, enum_keys))
            elif isinstance(val, (dict, list)):
                result.update(find_keys_used(val, user_keys, enum_keys))
    elif isinstance(obj, list):
        for item in obj:
            result.update(find_keys_used(item, user_keys, enum_keys))
    return result



def is_terminal_list(lst):
    """
    Check if all items in a list are of type float, int, or bool.
    
    Parameters:
        lst (list): The list of items to check.
    
    Returns:
        bool: True if all items are of type float, int, or bool, False
        otherwise.
    """
    return all(isinstance(item, (float, int, bool)) for item in lst)

def full_path_json(data, prefix='', sep='\x1f'):
    """
    Recursively traverses a JSON-like data structure and prepends a given prefix to each key, separated by a given separator.
    
    Parameters:
        data (dict): The JSON-like data structure to traverse.
        prefix (str, optional): The prefix to prepend to each key. Defaults to
            ''.
        sep (str, optional): The separator to use between the prefix and each
            key. Defaults to '\x1f'.
    
    Returns:
        dict: A new dictionary with the keys prepended by the prefix and
            separated by the separator.
    """
    # \x1f (\31): ASCII unit separator
    result = {}
    for key, value in data.items():
        new_key = f"{prefix}{sep}{key}" if prefix else key
        if isinstance(value, dict):
            result.update(full_path_json(value, new_key, sep))
        elif isinstance(value, list):
            if is_terminal_list(value):
                result[new_key] = np.ascontiguousarray(value)
            else:
                for i, item in enumerate(value):
                    if isinstance(item, dict):
                        result.update(full_path_json(
                            item, f"{new_key}{sep}{i}", sep))
                        #result.update(full_path_json(item, f"{new_key}[{i}]", sep))
                    else:
                        result[f"{new_key}{sep}{i}"] = item
                        #result[f"{new_key}[{i}]"] = item
        else:
            result[new_key] = value
    return result

def full_path_json_file(filename):
    """
    Reads a JSON file specified by the given filename and returns a new
    dictionary with the keys prepended by the prefix and separated by the
    separator.
    
    Parameters:
        filename (str): The path to the JSON file to be read.
    
    Returns:
        dict: A new dictionary with the keys prepended by the prefix and
            separated by the separator.
    """
    with open(filename, 'r') as file:
        json_data = json.load(file)
    return full_path_json(json_data)

def parse_parameter_json(filename):
    """
    Parse a JSON file containing simulation parameters and return a list with
    two elements.
    
    Parameters:
        filename (str): The path to the JSON file to be parsed.
    
    Returns:
        list: A list with two elements. The first element is a list of
        simulation parameters found in the JSON file. The second element is an
        empty dictionary.
    """
    with open(filename, 'r') as file:
        sims_json = json.load(file)
    param = find_keys_used(sims_json, user_keys=True, enum_keys=False)
    return [param,{}]

def parse_parameter_csv(filename, has_names=True, has_nominals=False):
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
        parameter={}
        nominal={}
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
            parameter[name] = []
        nominal = {name: None for name in names}
        # The next (second) line has the nominal values (if provided)
        if has_nominals:
            nominals = next(reader)
            for name, val in zip(names, nominals):
                nominal[name] = float(val)#TODO Check type of each val?
        for row_col in reader:
            for name, col in zip(names, row_col):
                parameter[name].append(float(col))
        parameter["sample_n"] = min(len(parameter[name])
                                            for name in names)
        if(parameter["sample_n"] > 1):
            # Convert lists to contiguous numpy arrays
            for key in parameter:
                parameter[key] = np.ascontiguousarray(
                    parameter[key])
        if(parameter["sample_n"] != max(len(parameter[name])
                                            for name in names)):
            warn('\n pym Jobs WARN Not all parameters have the same number '
                    +'of values. '
                    +'Something might be wrong with your CSV file.')
        return [parameter, nominal]
    
def parse_parameter_file(filename, has_names=True, has_nominals=False):
    """
    Parses a parameter file and returns a list containing a dictionary of parameter values and a dictionary of nominal values.
    
    Parameters:
        filename (str): The path to the parameter file.
        has_names (bool, optional): Whether the first line of the file contains
            parameter names. Defaults to True.
        has_nominals (bool, optional): Whether the second line of the file
            contains nominal values. Defaults to False.
    
    Returns:
        list: A list containing two dictionaries. The first dictionary contains
        the parameter values, where the keys are the parameter names and the
        values are the corresponding values. The second dictionary contains the
        nominal values, where the keys are the parameter names and the values
        are the corresponding nominal values.
    
    Raises:
        Exception: If the file extension is not recognized (i.e., not '.json' or
            '.csv').
    """
    # switch on extension
    ext = os.path.splitext(filename)[1] # extension with dot (e.g., '.csv')
    if ext == '.json':
        return parse_parameter_json(filename)
    if ext == '.csv':
        return parse_parameter_csv(filename, has_names, has_nominals)
    else:
        raise Exception(f'Unknown parameter file format: {filename}')

