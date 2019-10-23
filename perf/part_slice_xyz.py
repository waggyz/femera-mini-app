#!/usr/bin/python
# -*- coding: utf8 -*-

def part_slice_xyz( part_n, cpu_n ):
  ''' C,N,X,Y,Z are integers; N>>C, both given
      for C=40: (5*X) * (8*Y) * Z approx N
 
      try: X=Y=Z=S: (8*S) * (10*S) * (4..20)*S approx N
      try: X=Y=Z=S: (5*S) * (8*S) * (3..16)*S approx N
  '''
  factors = {
    1 : ((1,1),),
    2 : ((1,2),),
    4 : ((2,2),),
    6 : ((2,3),),
    8 : ((2,4),(4,6)),
    12: ((3,4),(4,6)),
    14: ((4,7),),
    16: ((4,4),),
    24: ((4,6),),
    28: ((4,7),),
    32: ((8,8),),
    40: ((5,8),(8,10)),
    64: ((8,8)),
    }
  dN=part_n
  for F in factors[cpu_n]:
    #print( 'Base Factors: ', F )
    try_f = list(range( int((F[0]+1)/2), F[1]*2+1 ))
    #print ('Try these:', try_f)
    NXYZ=set()
    NXYZ.add( (1, 1,1,1) )
    S=set()
    S.add(1)
    for F2 in try_f:
      s = int(round( ( part_n/F[0]/F[1]/F2 )**(1.0/3.0) ))
      n = F[0]*s * F[1]*s * F2*s
      S.add(s)
      NXYZ.add( ( n, F[0]*s,F[1]*s, F2*s ) )
    #print( 'S:', S, 'NXYZ:', NXYZ )
    for I in NXYZ:
      if abs( I[0] - part_n ) < dN:
        dN = abs( I[0] - part_n )
        best=(I[0],I[1],I[2],I[3])
  return(best)


if __name__ == "__main__":
  import argparse

  parser = argparse.ArgumentParser(description='Partition Slice Calculator')
  parser.add_argument("-n", "--part_n", help="Target number of partitions",required=True)
  parser.add_argument("-c", "--div_by", help="Number of processes",required=True)
  args = parser.parse_args()
  print( "%i %i %i %i"% part_slice_xyz( int(args.part_n), int(args.div_by) ) )
  
  ''' Integration Test '''
  #print( "%i,%i,%i,%i\n"% part_slice_xyz( 1000, 40 ) )
