#include <valarray>
#include "femera.h"
#include "test.h"
int Test::CheckCubeError( RESTRICT Phys::vals &errors, FLOAT_PHYS nu,
  const RESTRICT Phys::vals coor, const RESTRICT Phys::vals disp){
  // Check simple tension displacement solution
  //INT_ORDER  apply_dir  = 0;//FIXED DOF direction 0,1,2
  // Actually, it's probably easier to normalize coor and disp, instead.
  FLOAT_PHYS apply_mag  = 1.0;//FIXME Can remove these now.
  RESTRICT Phys::vals size_xyz={1.0,1.0,1.0};
  //
  INT_ORDER Dm=3, Dn=3;
  if(disp.size() > coor.size()){ Dn=4; }
  FLOAT_PHYS u[Dn], e[Dn+1], umin[Dn+1], uavg[Dn+1], umax[Dn+1],unrm=0.0,e2;
  for(INT_ORDER i=0;i<(Dn+1);i++){
    umin[i]= 99e99; uavg[i]=0.0; umax[i]=-99e99; };
  //switch(errors.size()){
  //  case(4): size_xyz={errors[1],errors[2],errors[3]};
  //  //case(2): apply_dir=errors[1];
  //  case(1): apply_mag=errors[0];
  //};
  //errors.resize(3*(Dn+1)+1);
  //
  INT_MESH node_n=disp.size()/Dn;
  //RESTRICT Phys::vals t;
  //t = coor[std::slice(0,node_n,Dn)]; size_xyz[0]=t.max()-t.min();
  //t = coor[std::slice(1,node_n,Dn)]; size_xyz[1]=t.max()-t.min();
  //t = coor[std::slice(2,node_n,Dn)]; size_xyz[2]=t.max()-t.min();
  //printf("SIZE: ");
  //for(uint i=0;i<uint(Dn);i++){printf("%f ",size_xyz[i]); }; printf("\n");
  //  
  for(size_t i=0;i<node_n;i++){
    u[0] = apply_mag*coor[Dm* i+0]/size_xyz[0];
    e[0] = disp[Dn *i+0]-u[0]; e[0]*=e[0]; uavg[0]+=e[0];
    if(e[0]<umin[0]){ umin[0]=e[0]; }; if(e[0]>umax[0]){ umax[0]=e[0]; };
    //
    u[1] =-apply_mag*coor[Dm* i+1]/size_xyz[1] * nu;
    e[1] = disp[Dn *i+1]-u[1]; e[1]*=e[1]; uavg[1]+=e[1];
    if(e[1]<umin[1]){ umin[1]=e[1]; }; if(e[1]>umax[1]){ umax[1]=e[1]; };
    //
    u[2] =-apply_mag*coor[Dm* i+2]/size_xyz[2] * nu;
    e[2] = disp[Dn *i+2]-u[2]; e[2]*=e[2]; uavg[2]+=e[2];
    if(e[2]<umin[2]){ umin[2]=e[2]; }; if(e[2]>umax[2]){ umax[2]=e[2]; };
    //
    e2=e[0]*e[0]+e[1]*e[1]+e[2]*e[2]; unrm+=e2;
    // Fourth element is displacement magnitude
    e[3] = std::sqrt(e2); uavg[3]+=e[3];
    if(e[3]<umin[3]){ umin[3]=e[3]; }; if(e[3]>umax[3]){ umax[3]=e[3]; };
    if(Dn>Dm){
      u[3] = 1.0;
      e[4] = disp[Dn *i+3]-u[3]; e[4]*=e[4]; uavg[4]+=e[4];
      if(e[4]<umin[4]){ umin[4]=e[4]; }; if(e[4]>umax[4]){ umax[4]=e[4]; };
      unrm+=e[4]*e[4];
    }
  };
  for(INT_ORDER i=0;i<(Dn+1);i++){ uavg[i]/=double(node_n); };
  if(Dn>Dm){
    errors={
      umin[0], umin[1], umin[2], umin[3], umin[4],
      uavg[0], uavg[1], uavg[2], uavg[3], uavg[4],
      umax[0], umax[1], umax[2], umax[3], umax[4],
      unrm };//FIXME
  }else{
    errors={
      umin[0], umin[1], umin[2], umin[3],
      uavg[0], uavg[1], uavg[2], uavg[3],
      umax[0], umax[1], umax[2], umax[3],
      unrm };//FIXME
  }
  return 0;
  };

