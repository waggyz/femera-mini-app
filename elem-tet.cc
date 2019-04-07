#include "femera.h"
//------------------------- Tets --------------------------
int Tet::JacsDets(){ return Jac3Dets(); };//FIXME
const RESTRICT Mesh::vals Tet::GaussLegendre(const INT_ORDER p ){
  //this->gaus_p = p;
  // Gauss-Legendre Tetrahedral Integration Points
  //FIXME For linear perturbations (edge nodes are interpolated),
  //FIXME the Jacobian is constant and independent of the int pt locations.
  //FIXME So only ONE 3x3+1 (Jacobian+det) is needed for each element
  //FIXME regardless of tet element order.
  //FIXME Volume of natural tet is 1/6.
  switch(int(p)){// { x,y,z, w ; ...}
  //case(0) :
  case(1):{ this->gaus_n= 1;
    return Mesh::vals{ 0.25,0.25,0.25, 1.0/6.0 };
    break;}
  case(2):{ this->gaus_n= 4;
    const FLOAT_MESH a=0.5854101966249685;// (5.0+3.0*std::sqrt(5.))/20.0;
    const FLOAT_MESH b=0.1381966011250105;// (5.0-std::sqrt(5.))/20.0;
    return Mesh::vals{
      a,b,b, 0.25/6.0,
      b,a,b, 0.25/6.0,
      b,b,a, 0.25/6.0,
      b,b,b, 0.25/6.0}; break;}
#if 0
  case(-3):{
    this->gaus_n= 5;//FIXME tet20s don't converge
    return Mesh::vals{//FIXME Triple-checked these 5-point rule values
      0.25 , 0.25 , 0.25 ,-4./ 30.,
      0.5  , 1./6., 1./6., 9./120.,
      1./6., 0.5  , 1./6., 9./120.,
      1./6., 1./6., 0.5  , 9./120.,
      1./6., 1./6., 1./6., 9./120.}; break;}
#endif
  case(3):{ this->gaus_n=11;//FIXME This converges tet20 meshes
    const FLOAT_MESH a=0.3994035761667992;// (1.+std::sqrt(5./14.))/4.;
    const FLOAT_MESH b=0.1005964238332008;// (1.-std::sqrt(5./14.))/4.;
    return Mesh::vals{
      0.25  , 0.25  , 0.25  , -74./ 5625.,
     11./14., 1./14., 1./14., 343./45000.,
      1./14.,11./14., 1./14., 343./45000.,
      1./14., 1./14.,11./14., 343./45000.,
      1./14., 1./14., 1./14., 343./45000.,
      a,a,b, 56./2250.,
      a,b,a, 56./2250.,
      a,b,b, 56./2250.,
      b,a,a, 56./2250.,
      b,a,b, 56./2250.,
      b,b,a, 56./2250.}; break;}
  default :{ return Mesh::vals{};}
  };
};
const RESTRICT Mesh::vals Tet::ShapeFunction(
  const INT_ORDER p, const FLOAT_MESH x[3]){
  const FLOAT_MESH v=1.0;//0.550321208149104;// vol of unit-sized tet is 1/6
  switch(int(p)){
  case(1): return Mesh::vals {v-v*x[0]-v*x[1]-v*x[2], v*x[0], v*x[1], v*x[2]};
  case(2):{ RESTRICT Mesh::vals f(10);
    const FLOAT_MESH L2=x[0]*v, L3=x[1]*v, L4=x[2]*v;
    const FLOAT_MESH L1=(1.-L2-L3-L4);
    f[ 0] = 2.*L1*L1 - L1;// corner nodes
    f[ 1] = 2.*L2*L2 - L2;
    f[ 2] = 2.*L3*L3 - L3;
    f[ 3] = 2.*L4*L4 - L4;
    f[ 4] = 4.*L2*L1 ;// Edge nodes
    f[ 5] = 4.*L2*L3 ;
    f[ 6] = 4.*L3*L1 ;
    f[ 7] = 4.*L4*L1 ;
    f[ 8] = 4.*L3*L4 ;
    f[ 9] = 4.*L4*L2 ;
    return f;}
  case(3):{ RESTRICT Mesh::vals f(20);
    const FLOAT_MESH L2=x[0]*v, L3=x[1]*v, L4=x[2]*v;
    const FLOAT_MESH L1=(1.-L2-L3-L4);
    f[ 0]= 0.5* L1 *(3.* L1 -1.)*(3* L1 -2.);// corner nodes;
    f[ 1]= 0.5* L2 *(3.* L2 -1.)*(3* L2 -2.);
    f[ 2]= 0.5* L3 *(3.* L3 -1.)*(3* L3 -2.);
    f[ 3]= 0.5* L4 *(3.* L4 -1.)*(3* L4 -2.);
    // edge nodes
    f[ 4]=4.5*( L1*L2 *(3.*L1 -1.) );
    f[ 5]=4.5*( L1*L2 *(3.*L2 -1.) );
    //
    f[ 6]=4.5*( L2*L3 *(3.*L2 -1.) );
    f[ 7]=4.5*( L2*L3 *(3.*L3 -1.) );
    //
    f[ 8]=4.5*( L1*L3 *(3.*L3 -1.) );
    f[ 9]=4.5*( L1*L3 *(3.*L1 -1.) );
    //
    f[10]=4.5*( L1*L4 *(3.*L4 -1.) );
    f[11]=4.5*( L1*L4 *(3.*L1 -1.) );
    //
    f[12]=4.5*( L3*L4 *(3.*L4 -1.) );
    f[13]=4.5*( L3*L4 *(3.*L3 -1.) );
    //
    f[14]=4.5*( L2*L4 *(3.*L4 -1.) );
    f[15]=4.5*( L2*L4 *(3.*L2 -1.) );
    // face nodes 0,1,2, 0,1,3, 0,3,2, 1,2,3 
    f[16]=27.*( L1*L2 *L3 );
    f[17]=27.*( L1*L2 *L4 );
    f[18]=27.*( L1*L4 *L3 );
    f[19]=27.*( L2*L3 *L4 );
    return f;}
  default: return Mesh::vals{};
  };
};
const RESTRICT Mesh::vals Tet::ShapeGradient(
  const INT_ORDER p, const FLOAT_MESH x[3]){
  RESTRICT Mesh::vals g={};
  //printf("=== GA ===\n");
  //FIXME Should these take a list of points?
  FLOAT_MESH v=1.0;//0.550321208149104;// (1/6)^(1/3)//FIXME?
  switch( int(p) ){//this->elem_p
  case(1):{// printf("=== G1 ===\n"); 
    //return Mesh::vals {//FIXME Transpose?
    g.resize(12);
    g={//FIXME Transpose?
      -v, v, 0.0, 0.0,  // dN/dx (natural coords)//FIXED Check these.
      -v, 0.0, v, 0.0,  // dN/dy
      -v, 0.0, 0.0, v }; break;}// dN/
  case(2):{ v=1.0;//FIXME?
    const FLOAT_MESH L2=x[0]*v, L3=x[1]*v, L4=x[2]*v;
    const FLOAT_MESH L1=(1.-L2-L3-L4);
    // Term-by-term derivs
    const FLOAT_MESH L1r=-v, L2r=v , L3r=0., L4r=0.;
    const FLOAT_MESH L1s=-v, L2s=0., L3s=v , L4s=0.;
    const FLOAT_MESH L1t=-v, L2t=0., L3t=0., L4t=v ;
    // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
    //RESTRICT Mesh::vals g(3*10);
    // r-derivs by product rule
    g.resize(30);
    g[ 0] = 2.*L1*L1r + 2.*L1r*L1 - L1r;// Corner nodes
    g[ 1] = 2.*L2*L2r + 2.*L2r*L2 - L2r;
    g[ 2] = 2.*L3*L3r + 2.*L3r*L3 - L3r;
    g[ 3] = 2.*L4*L4r + 2.*L4r*L4 - L4r;
    g[ 4] = 4.*L2*L1r + 4.*L2r*L1;// Edge nodes
    g[ 5] = 4.*L2*L3r + 4.*L2r*L3;
    g[ 6] = 4.*L3*L1r + 4.*L3r*L1;
    g[ 7] = 4.*L4*L1r + 4.*L4r*L1;
    g[ 8] = 4.*L3*L4r + 4.*L3r*L4;
    g[ 9] = 4.*L4*L2r + 4.*L4r*L2;
    // s-derivs
    g[10] = 2.*L1*L1s + 2.*L1s*L1 - L1s;// Corner nodes
    g[11] = 2.*L2*L2s + 2.*L2s*L2 - L2s;
    g[12] = 2.*L3*L3s + 2.*L3s*L3 - L3s;
    g[13] = 2.*L4*L4s + 2.*L4s*L4 - L4s;
    g[14] = 4.*L2*L1s + 4.*L2s*L1;// Edge nodes
    g[15] = 4.*L2*L3s + 4.*L2s*L3;
    g[16] = 4.*L3*L1s + 4.*L3s*L1;
    g[17] = 4.*L4*L1s + 4.*L4s*L1;
    g[18] = 4.*L3*L4s + 4.*L3s*L4;
    g[19] = 4.*L4*L2s + 4.*L4s*L2;
    // t-derivs
    g[20] = 2.*L1*L1t + 2.*L1t*L1 - L1t;// Corner nodes
    g[21] = 2.*L2*L2t + 2.*L2t*L2 - L2t;
    g[22] = 2.*L3*L3t + 2.*L3t*L3 - L3t;
    g[23] = 2.*L4*L4t + 2.*L4t*L4 - L4t;
    g[24] = 4.*L2*L1t + 4.*L2t*L1;// Edge nodes
    g[25] = 4.*L2*L3t + 4.*L2t*L3;
    g[26] = 4.*L3*L1t + 4.*L3t*L1;
    g[27] = 4.*L4*L1t + 4.*L4t*L1;
    g[28] = 4.*L3*L4t + 4.*L3t*L4;
    g[29] = 4.*L4*L2t + 4.*L4t*L2;
    //
    //return g;
    break;}
  case(3):{ v=1.0;
    const FLOAT_MESH L2=x[0]*v, L3=x[1]*v, L4=x[2]*v;
    const FLOAT_MESH L1=(1.-L2-L3-L4);
    // Term-by-term derivs
    const FLOAT_MESH L1r=-v, L2r=v , L3r=0., L4r=0.;
    const FLOAT_MESH L1s=-v, L2s=0., L3s=v , L4s=0.;
    const FLOAT_MESH L1t=-v, L2t=0., L3t=0., L4t=v ;
    // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
    //RESTRICT Mesh::vals g(3*20);
    g.resize(60);
    g[ 0]= 0.5* L1r *(3.* L1 -1.)*(3.* L1 -2.)// corner nodes
         + 0.5* L1  *(3.* L1r   )*(3.* L1 -2.)
         + 0.5* L1  *(3.* L1 -1.)*(3.* L1r   );
    g[ 1]= 0.5* L2r *(3.* L2 -1.)*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2r   )*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2 -1.)*(3.* L2r   );
    g[ 2]= 0.5* L3r *(3.* L3 -1.)*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3r   )*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3 -1.)*(3.* L3r   );
    g[ 3]= 0.5* L4r *(3.* L4 -1.)*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4r   )*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4 -1.)*(3.* L4r   );
    // r-derivs, edge nodes
    g[ 4]=4.5*( L1r*L2 *(3.*L1 -1.) + L1 *L2r*(3.*L1 -1.) + L1 *L2 *(3.*L1r) );
    g[ 5]=4.5*( L1r*L2 *(3.*L2 -1.) + L1 *L2r*(3.*L2 -1.) + L1 *L2 *(3.*L2r) );
    //
    g[ 6]=4.5*( L2r*L3 *(3.*L2 -1.) + L2 *L3r*(3.*L2 -1.) + L2 *L3 *(3.*L2r) );
    g[ 7]=4.5*( L2r*L3 *(3.*L3 -1.) + L2 *L3r*(3.*L3 -1.) + L2 *L3 *(3.*L3r) );
    //
    g[ 8]=4.5*( L1r*L3 *(3.*L3 -1.) + L1 *L3r*(3.*L3 -1.) + L1 *L3 *(3.*L3r) );
    g[ 9]=4.5*( L1r*L3 *(3.*L1 -1.) + L1 *L3r*(3.*L1 -1.) + L1 *L3 *(3.*L1r) );
    //
    g[10]=4.5*( L1r*L4 *(3.*L4 -1.) + L1 *L4r*(3.*L4 -1.) + L1 *L4 *(3.*L4r) );
    g[11]=4.5*( L1r*L4 *(3.*L1 -1.) + L1 *L4r*(3.*L1 -1.) + L1 *L4 *(3.*L1r) );
    //
    g[12]=4.5*( L3r*L4 *(3.*L4 -1.) + L3 *L4r*(3.*L4 -1.) + L3 *L4 *(3.*L4r) );
    g[13]=4.5*( L3r*L4 *(3.*L3 -1.) + L3 *L4r*(3.*L3 -1.) + L3 *L4 *(3.*L3r) );
    //
    g[14]=4.5*( L2r*L4 *(3.*L4 -1.) + L2 *L4r*(3.*L4 -1.) + L2 *L4 *(3.*L4r) );
    g[15]=4.5*( L2r*L4 *(3.*L2 -1.) + L2 *L4r*(3.*L2 -1.) + L2 *L4 *(3.*L2r) );
    // face nodes 0,1,2, 0,1,3, 0,3,2, 1,2,3 
    g[16]=27.*( L1r*L2 *L3 + L1 *L2r*L3 + L1 *L2 *L3r);
    g[17]=27.*( L1r*L2 *L4 + L1 *L2r*L4 + L1 *L2 *L4r);
    g[18]=27.*( L1r*L4 *L3 + L1 *L4r*L3 + L1 *L4 *L3r);
    g[19]=27.*( L2r*L3 *L4 + L2 *L3r*L4 + L2 *L3 *L4r);
    // s-derivs
    g[20]= 0.5* L1s *(3.* L1 -1.)*(3.* L1 -2.)// corner nodes
         + 0.5* L1  *(3.* L1s   )*(3.* L1 -2.)
         + 0.5* L1  *(3.* L1 -1.)*(3.* L1s   );
    g[21]= 0.5* L2s *(3.* L2 -1.)*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2s   )*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2 -1.)*(3.* L2s   );
    g[22]= 0.5* L3s *(3.* L3 -1.)*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3s   )*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3 -1.)*(3.* L3s   );
    g[23]= 0.5* L4s *(3.* L4 -1.)*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4s   )*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4 -1.)*(3.* L4s   );
    // s-desivs, edge nodes
    g[24]=4.5*( L1s*L2 *(3.*L1 -1.) + L1 *L2s*(3.*L1 -1.) + L1 *L2 *(3.*L1s) );
    g[25]=4.5*( L1s*L2 *(3.*L2 -1.) + L1 *L2s*(3.*L2 -1.) + L1 *L2 *(3.*L2s) );
    //
    g[26]=4.5*( L2s*L3 *(3.*L2 -1.) + L2 *L3s*(3.*L2 -1.) + L2 *L3 *(3.*L2s) );
    g[27]=4.5*( L2s*L3 *(3.*L3 -1.) + L2 *L3s*(3.*L3 -1.) + L2 *L3 *(3.*L3s) );
    //
    g[28]=4.5*( L1s*L3 *(3.*L3 -1.) + L1 *L3s*(3.*L3 -1.) + L1 *L3 *(3.*L3s) );
    g[29]=4.5*( L1s*L3 *(3.*L1 -1.) + L1 *L3s*(3.*L1 -1.) + L1 *L3 *(3.*L1s) );
    //
    g[30]=4.5*( L1s*L4 *(3.*L4 -1.) + L1 *L4s*(3.*L4 -1.) + L1 *L4 *(3.*L4s) );
    g[31]=4.5*( L1s*L4 *(3.*L1 -1.) + L1 *L4s*(3.*L1 -1.) + L1 *L4 *(3.*L1s) );
    //
    g[32]=4.5*( L3s*L4 *(3.*L4 -1.) + L3 *L4s*(3.*L4 -1.) + L3 *L4 *(3.*L4s) );
    g[33]=4.5*( L3s*L4 *(3.*L3 -1.) + L3 *L4s*(3.*L3 -1.) + L3 *L4 *(3.*L3s) );
    //
    g[34]=4.5*( L2s*L4 *(3.*L4 -1.) + L2 *L4s*(3.*L4 -1.) + L2 *L4 *(3.*L4s) );
    g[35]=4.5*( L2s*L4 *(3.*L2 -1.) + L2 *L4s*(3.*L2 -1.) + L2 *L4 *(3.*L2s) );
    // face nodes 0,1,2, 0,1,3, 0,3,2, 1,2,3 
    g[36]=27.*( L1s*L2 *L3 + L1 *L2s*L3 + L1 *L2 *L3s);
    g[37]=27.*( L1s*L2 *L4 + L1 *L2s*L4 + L1 *L2 *L4s);
    g[38]=27.*( L1s*L4 *L3 + L1 *L4s*L3 + L1 *L4 *L3s);
    g[39]=27.*( L2s*L3 *L4 + L2 *L3s*L4 + L2 *L3 *L4s);
    // t-derivs
    g[40]= 0.5* L1t *(3.* L1 -1.)*(3.* L1 -2.)// corner nodes
         + 0.5* L1  *(3.* L1t   )*(3.* L1 -2.)
         + 0.5* L1  *(3.* L1 -1.)*(3.* L1t   );
    g[41]= 0.5* L2t *(3.* L2 -1.)*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2t   )*(3.* L2 -2.)
         + 0.5* L2  *(3.* L2 -1.)*(3.* L2t   );
    g[42]= 0.5* L3t *(3.* L3 -1.)*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3t   )*(3.* L3 -2.)
         + 0.5* L3  *(3.* L3 -1.)*(3.* L3t   );
    g[43]= 0.5* L4t *(3.* L4 -1.)*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4t   )*(3.* L4 -2.)
         + 0.5* L4  *(3.* L4 -1.)*(3.* L4t   );
    // t-detivs, edge nodes
    g[44]=4.5*( L1t*L2 *(3.*L1 -1.) + L1 *L2t*(3.*L1 -1.) + L1 *L2 *(3.*L1t) );
    g[45]=4.5*( L1t*L2 *(3.*L2 -1.) + L1 *L2t*(3.*L2 -1.) + L1 *L2 *(3.*L2t) );
    //
    g[46]=4.5*( L2t*L3 *(3.*L2 -1.) + L2 *L3t*(3.*L2 -1.) + L2 *L3 *(3.*L2t) );
    g[47]=4.5*( L2t*L3 *(3.*L3 -1.) + L2 *L3t*(3.*L3 -1.) + L2 *L3 *(3.*L3t) );
    //
    g[48]=4.5*( L1t*L3 *(3.*L3 -1.) + L1 *L3t*(3.*L3 -1.) + L1 *L3 *(3.*L3t) );
    g[49]=4.5*( L1t*L3 *(3.*L1 -1.) + L1 *L3t*(3.*L1 -1.) + L1 *L3 *(3.*L1t) );
    //
    g[50]=4.5*( L1t*L4 *(3.*L4 -1.) + L1 *L4t*(3.*L4 -1.) + L1 *L4 *(3.*L4t) );
    g[51]=4.5*( L1t*L4 *(3.*L1 -1.) + L1 *L4t*(3.*L1 -1.) + L1 *L4 *(3.*L1t) );
    //
    g[52]=4.5*( L3t*L4 *(3.*L4 -1.) + L3 *L4t*(3.*L4 -1.) + L3 *L4 *(3.*L4t) );
    g[53]=4.5*( L3t*L4 *(3.*L3 -1.) + L3 *L4t*(3.*L3 -1.) + L3 *L4 *(3.*L3t) );
    //
    g[54]=4.5*( L2t*L4 *(3.*L4 -1.) + L2 *L4t*(3.*L4 -1.) + L2 *L4 *(3.*L4t) );
    g[55]=4.5*( L2t*L4 *(3.*L2 -1.) + L2 *L4t*(3.*L2 -1.) + L2 *L4 *(3.*L2t) );
    // face nodes 0,1,2, 0,1,3, 0,3,2, 1,2,3 
    g[56]=27.*( L1t*L2 *L3 + L1 *L2t*L3 + L1 *L2 *L3t);
    g[57]=27.*( L1t*L2 *L4 + L1 *L2t*L4 + L1 *L2 *L4t);
    g[58]=27.*( L1t*L4 *L3 + L1 *L4t*L3 + L1 *L4 *L3t);
    g[59]=27.*( L2t*L3 *L4 + L2 *L3t*L4 + L2 *L3 *L4t);
    //
    //return g;
    break;}
  //default: return Mesh::vals{};
  default: break;//const RESTRICT Mesh::vals g={};
  };
  return g;
  // Transpose it
  //RESTRICT Mesh::vals t(g.size());
  //int r = int( g.size()/3 );
  //for(int i=0;i<r;i++){
  //  for(int j=0;j<3;j++){
  //    t[3* j+i ] = g[r* i+j ];
  //};};
  //return t;
  };
