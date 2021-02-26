#!/usr/bin/python
# -*- coding: utf8 -*-
import os
import sys
sys.path.append('tools/testy')
import testy
import check_code_graffiti as cg
import re

def testy_exe_test( *args, **opts ):
  if len(sys.argv) >1:
    # sys.argv[ 0 ] contains the path and name of this file.
    src_dir = sys.argv[ 1 ]# first argument is executable name.
    if len(sys.argv) > 2:
      build_dir = sys.argv[ 2 ]# optional second argument is build directory.
    else:
      build_dir = src_dir
    #
    suite = testy.Suite('Check CPU model and capabilities',
      because='Femera needs valid CPU model name and capabilities',
      quiet_check_pass=True )
    #
    exe = 'tools/cpucount.sh'
    check = testy.Exe([ exe ],shell=True ).run()
    if int(check.stdout) <= 0:
      check.will('WARN', because=exe+' counted no cores.')
    else:
      check.will('INFO',
        because=exe+' counted '+check.stdout+' physical cores.')
    check.done( suite )
    #
    exe = 'tools/cpumodel.sh'
    check = testy.Exe([ exe ],shell=True ).run()
    n = sum(c.isalpha() for c in check.stdout)
    n+= sum(c.isdigit() for c in check.stdout)
    if n  > 3:
      check.will('info',
        because=exe+': \"'+check.stdout+'\" has more than 3 alphanumeric.')
    else:
      check.will('note',
        because=exe+' contains '+check.stdout+', < 3 alphanumeric.')
    check.done( suite )
    #
    exe = 'tools/cpusimd.sh'
    check = testy.Exe([ exe ],shell=True ).run()
    if check.stdout == "" :
      check.will('note',
        because=exe+': found no floating point CPU SIMD capabilities.')
      check.will('note',
        because='Femera will use only scalar operations.')
      check.done( suite )
    else:
      check.will('info', because=exe+': '+check.stdout+'.')
      check.done( suite )
      if 'AVX' in check.stdout:
        check.will('PASS', because='CPU supports AVX.')
      else:
        check.will('note',
          because='Femera will use only scalar or auto-vectorized operations.')
      check.done( suite )
    #
    suite.done()
    #
    suite = testy.Suite('Check compiler capabilities',
      because='Femera uses compiler capabilities available.',
      quiet_check_pass=True )
    #
    with open(build_dir+'/CXXFLAGS') as f:
      cxxflags = f.read().split()
    cxx = ['g++']+['-x']+['c++']+cxxflags+['-']+['-o']
    #
    exe=[cxx[0],'--version']
    check = testy.Exe( exe ).run()
    if check.retval != 0:
      check.will('FAIL', because=exe[0]+' returned '+str(check.retval)+'.')
    else:
      check.will('info', because='compiler is '+check.stdout.split("\n")[0]+'.')
    check.done( suite )
    #
    fname='pragma_simd_test'
    code = '''
      #include <stdio.h>
      int main(){
        int a=-6;
      #pragma omp simd
        for(int i=0;i<4; i++){ a+=i; };
        printf(\"%i\\n\", a );
        return 0; }
      '''
    exe=build_dir+'/'+fname
    check = testy.Exe( cxx+[exe], stdin=code ).run()
    if check.retval != 0:
      check.will('FAIL', because=': compiler returned '+str(check.retval)+'.')
      check.done( suite )
    else:
      check.will('PASS', because=fname+': compiled.')
      check.done( suite )
      if len(check.stderr) > 0:
        check.will('info', 'compiler does not support #pragma omp simd.')
      else:
        check.will('pass',
          because='compiler supports #pragma omp simd.')
      check.done( suite )
    #
    suite.done()
    #
    suite = testy.Suite('Check Femera version number format',
      because='Semantic Versioning: https://semver.org/',
      quiet_check_pass=True )
    #
    check_release = testy.Exe([ 'cat','data/VERSION' ] ).run()
    release=check_release.stdout
    if check_release.retval == 0 :
      check_release.will('pass', because='data/VERSION found.')
    else:
      check_release.will('FAIL', because='data/VERSION NOT found.')
    check_release.done( suite )
    #
    if re.match('^[0-9]*\.[0-9]*$', release ) :
      check_release.will('pass', because='Version '+release+' is x.y.')
    else:
      check_release.will('FAIL', because='data/VERSION is NOT x.y.')
    check_release.done( suite )
    #
    check_pre = testy.Exe([ 'cat','data/PRE_RELEASE' ] ).run()
    pre_release=check_pre.stdout
    if check_pre.retval == 0 :
      check_pre.will('pass', because='data/PRE_RELEASE found.')
    else:
      check_pre.will('FAIL', because='data/PRE_RELEASE NOT found.')
    check_pre.done(suite)
    #
    if check_pre.stdout =="":
      check_pre.will('pass', because='data/PRE_RELEASE is empty.')
      check_pre.done(suite)
    else:
      check_pre.will('info','pre-release is: '+check_pre.stdout+'.')
      check_pre.done(suite)
      if re.match('^[a-z0-9\.-]+$', pre_release ) :
        check_pre.will('PASS','pre-release contains only alphanum-.')
      else:
        check_pre.will('WARN','pre-release contains characters not alphanum-.')
      check_pre.done(suite)
      #
      if( pre_release.startswith( 'alpha') or pre_release.startswith( 'beta')
         or pre_release.startswith( 'rc') ):
        check_pre.will('pass',
          because='pre-relase starts with \"alpha\" or \"beta\".')
      else:
        check_pre.will('WARN',
          because='pre-relase does NOT start with \"alpha\" or \"beta\".')
      check_pre.done( suite )
    #
    suite.done()
    #
  return

if __name__ == "__main__":
  ''' Testy pre-build tests '''
  # Check myself for graffiti
  cg.check_graffiti( sys.argv[0] )
  #
  testy_exe_test()
