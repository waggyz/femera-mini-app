#!/usr/bin/env python3
from pymera_enum import fmr_enum, Data_type, Grid_structure

import numpy as np
import json

def find_user_keys(obj):
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
            if key in fmr_enum:
                result[key] = fmr_enum[key][val.upper()]
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
                    result.update(find_user_keys(val))
            elif isinstance(val, (dict, list)):
                result.update(find_user_keys(val))
    elif isinstance(obj, list):
        for item in obj:
            result.update(find_user_keys(item))
    return result

def parse_sims_from_file(sims, filename):
    #sims = Sims(self)
    with open(filename, 'r') as file:
        sims_json = json.load(file)#TODO use json()
    if 'fmr:Sims' in sims_json:
        sims_json = sims_json['fmr:Sims']
        sims.parameter = find_user_keys(sims_json)
        #
        if 'fmr:Name' in sims_json:
            sims.set_name(sims_json['fmr:Name'])
        if 'fmr:Version' in sims_json:
            sims.set_version(sims_json['fmr:Version'])
    else:
        print('Found no fmr:Sims in ' + filename)
        return
    #if 'fmr:Data:Name' in sims_json:
    #    sims.name=sims_json['fmr:Data:Name']
    #self.add_sims(name=sims_json['name'], runs_n=sims_json['runs_n'])
    return sims
'''
def flatten_json(data, prefix=''):
    result = {}
    for key, value in data.items():
        new_key = f"{prefix}|{key}" if prefix else key
        if isinstance(value, dict):
            result.update(flatten_json(value, new_key))
        elif isinstance(value, list):
            for i, item in enumerate(value):
                if isinstance(item, dict):
                    result.update(flatten_json(item, f"{new_key}|{i}"))
                else:
                    result[f"{new_key}|{i}"] = item
        else:
            result[new_key] = value
    return result
'''


def is_terminal_list(lst):
    return all(isinstance(item, (float, int, bool)) for item in lst)

def flatten_json(data, prefix='', sep='\x1f'):
    # \x1f (\31): ASCII unit separator
    result = {}
    for key, value in data.items():
        new_key = f"{prefix}{sep}{key}" if prefix else key
        if isinstance(value, dict):
            result.update(flatten_json(value, new_key, sep))
        elif isinstance(value, list):
            if is_terminal_list(value):
                result[new_key] = np.ascontiguousarray(value)
            else:
                for i, item in enumerate(value):
                    if isinstance(item, dict):
                        result.update(flatten_json(
                            item, f"{new_key}{sep}{i}", sep))
                        #result.update(flatten_json(item, f"{new_key}[{i}]", sep))
                    else:
                        result[f"{new_key}{sep}{i}"] = item
                        #result[f"{new_key}[{i}]"] = item
        else:
            result[new_key] = value
    return result
'''
def flatten_json(data, prefix='', index=0):
    result = {}
    if isinstance(data, dict):
        for key, value in data.items():
            new_key = f"{prefix}[{index}]|{key}" if prefix else key
            if isinstance(value, dict):
                result.update(flatten_json(value, new_key))
            elif isinstance(value, list):
                if is_terminal_list(value):
                    result[new_key] = value
                else:
                    for i, item in enumerate(value):
                        result.update(flatten_json(item, new_key, i))
            else:
                result[new_key] = value
    elif isinstance(data, list):
        for i, item in enumerate(data):
            result.update(flatten_json(item, prefix, i))
    else:
        result[f"{prefix}[{index}]"] = data
    return result
'''
def flatten_json_file(filename):
    with open(filename, 'r') as file:
        json_data = json.load(file)
    return flatten_json(json_data)


