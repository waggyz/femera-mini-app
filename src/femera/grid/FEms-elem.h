namespace femera { namespace grid { namespace fems {

//================================= points ===================================
static constexpr fmr::Local_int vert_d      = 0;
static constexpr fmr::Local_int node_d      = 0;

//================================== bars ====================================
static constexpr fmr::Local_int bars_d      = 1;// bars spatial dimension
static constexpr fmr::Local_int bars_vert_n = 2;
static constexpr
fmr::Local_int  bars_vert_conn [bars_vert_n] = {0,1};
//
static constexpr fmr::Geom_float bars_meas  = 2.0;// natural element length
static constexpr
fmr::Geom_float bars_vert_coor [bars_vert_n * bars_d ]// transposed coor_vert
  = {
       // vertex                      //
  -1.0,// 0       0---1               //
   1.0 // 1                o--x       //
};
//================================== tris ====================================
static constexpr fmr::Local_int tris_d      = 2;// tris spatial dimension
static constexpr fmr::Local_int tris_vert_n = 3;
static constexpr fmr::Local_int tris_edge_n = 3;
static constexpr
fmr::Local_int tris_vert_conn [tris_vert_n] = {0,1,2};
static constexpr
fmr::Local_int tris_vert_edge_bars [bars_vert_n * tris_edge_n]
  = {
  0,1, 1,2, 2,0
};
static constexpr fmr::Geom_float tris_meas  = 0.5;// natural element area
static constexpr
fmr::Geom_float tris_vert_coor [tris_vert_n * tris_d ]// transposed coor_vert
  = {
           // vertex  2              //
  0.0, 0.0,// 0       |\      y      //
  1.0, 0.0,// 1       | \     |      //
  0.0, 1.0 // 2       0--1    o--x   //
};
//================================== quad ====================================
static constexpr fmr::Local_int quad_d      = 2;// tris spatial dimension
static constexpr fmr::Local_int quad_vert_n = 4;
static constexpr fmr::Local_int quad_edge_n = 4;
static constexpr
fmr::Local_int quad_vert_conn [quad_vert_n] = {0,1,2,4};
static constexpr
fmr::Local_int quad_vert_edge_bars [bars_vert_n * quad_edge_n]
  = {
  0,1, 1,2, 2,3, 3,0
};
static constexpr fmr::Geom_float quad_meas  = 4.0;// natural element area
static constexpr
fmr::Geom_float quad_vert_coor [quad_vert_n * quad_d ]// transposed coor_vert
  = {
            // vertex                //
  -1.0,-1.0,// 0      3----2         //
   1.0,-1.0,// 1      |    |    y    //
   1.0, 1.0,// 2      |    |    |    //
  -1.0, 1.0 // 3      0----1    o--x //
};
static constexpr
fmr::Geom_float quad_coor_vert [quad_d * quad_vert_n]// transposed vert_coor
  = {
  -1.0, 1.0, 1.0,-1.0,
  -1.0,-1.0, 1.0, 1.0,
};
//================================== tets ====================================
static constexpr fmr::Local_int tets_d      = 3;// tets spatial dimension
static constexpr fmr::Local_int tets_vert_n = 4;
static constexpr fmr::Local_int tets_edge_n = 6;
static constexpr fmr::Local_int tets_face_n = 4;
static constexpr
  fmr::Local_int tets_vert_conn [tets_vert_n] = {0,1,2,3};
//
//NOTE alternate tets vertex numbering convention below.
//const FMR_RESTRICT Mesh::ints vert_edge={ 0,1, 1,2, 2,0, 0,3, 1,3, 2,3 };
//const FMR_RESTRICT Mesh::ints vert_face={ 0,1,2, 0,1,3, 1,2,3, 2,0,3 }
//
//NOTE The following match the gmsh convention for tets.
static constexpr
fmr::Local_int tets_vert_edge_bars [bars_vert_n * tets_edge_n]
  = {
  0,1, 1,2, 2,0, 0,3, 2,3, 1,3
};
static constexpr
fmr::Local_int tets_vert_face_tris [tris_vert_n * tets_face_n]
  = { //NOTE the normals point inward. This might be wrong.
  0,1,2, 0,3,1, 0,2,3, 1,3,2
};
static constexpr fmr::Geom_float tets_meas  = 1.0 / 6.0;// ntrl elem volume
static constexpr fmr::Geom_float tets_face_meas  = 2.0;// elem surface area
static constexpr
fmr::Geom_float tets_vert_coor [tets_vert_n * tets_d ]// transposed coor_vert
  = {            //            3               //
                 // vertex    /|\              //
  0.0, 0.0, 0.0, // 0        / | \             //
  1.0, 0.0, 0.0, // 1       2--|--1            //
  0.0, 1.0, 0.0, // 2        \ | /     y z x   //
  0.0, 0.0, 1.0  // 3         \|/       \|/    //
};               //            0         o     //
static constexpr
fmr::Geom_float tets_coor_vert [tets_d * tets_vert_n]// transposed vert_coor
  = {
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0
};
//-------------------------- tet shape functions -----------------------------
#if 0
static inline
const FMR_RESTRICT Mesh::vals Tet::ShapeFunction(
  const fmr::Dim_int p, const fmr::Float_phys x[3]){
  const fmr::Float_phys v=1.0;//0.550321208149104;
  // vol of unit-sized tet is 1/6
  switch(int(p)){
  case(1): return Mesh::vals {v-v*x[0]-v*x[1]-v*x[2], v*x[0], v*x[1], v*x[2]};
  case(2):{ FMR_RESTRICT Mesh::vals f(10);
    const fmr::Float_phys L2=x[0]*v, L3=x[1]*v, L4=x[2]*v;
    const fmr::Float_phys L1=(v-L2-L3-L4);
    f[ 0] = 2.*L1*L1 - L1;// corner nodes
    f[ 1] = 2.*L2*L2 - L2;
    f[ 2] = 2.*L3*L3 - L3;
    f[ 3] = 2.*L4*L4 - L4;
    f[ 4] = 4.*L1*L2 ;// Edge nodes
    f[ 5] = 4.*L2*L3 ;
    f[ 6] = 4.*L3*L1 ;
    f[ 7] = 4.*L1*L4 ;
    f[ 8] = 4.*L3*L4 ;
    f[ 9] = 4.*L2*L4 ;
    return f; }
  case(3):{ FMR_RESTRICT Mesh::vals f(20);//TODO 16-node cubic tet
    const fmr::Float_phys L2=x[0]*v, L3=x[1]*v, L4=x[2]*v;
    const fmr::Float_phys L1=(v-L2-L3-L4);
    f[ 0]= 0.5* L1 *(3.* L1 -1.)*(3.* L1 -2.);// corner nodes;
    f[ 1]= 0.5* L2 *(3.* L2 -1.)*(3.* L2 -2.);
    f[ 2]= 0.5* L3 *(3.* L3 -1.)*(3.* L3 -2.);
    f[ 3]= 0.5* L4 *(3.* L4 -1.)*(3.* L4 -2.);
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
}
static inline
const FMR_RESTRICT Mesh::vals Tet::ShapeGradient(
  const fmr::Dim_int p, const fmr::Float_phys x[3]) {
  FMR_RESTRICT Mesh::vals g={};
  //printf("=== GA ===\n");
  //FIXME Should these take a list of points?
  fmr::Float_phys v=1.0;//0.550321208149104;// (1/6)^(1/3)//FIXME?
  switch( int(p) ){//this->elem_p
  case(1):{// printf("=== G1 ===\n"); 
    //return Mesh::vals {//FIXME Transpose?
    g.resize(12);
    g={//FIXME Transpose?
      -v, v, 0.0, 0.0,  // dN/dx (natural coords)//FIXED Check these.
      -v, 0.0, v, 0.0,  // dN/dy
      -v, 0.0, 0.0, v }; break;}// dN/
  case(2):{ v=1.0;//FIXME?
    const fmr::Float_phys L2=x[0]*v, L3=x[1]*v, L4=x[2]*v;
    const fmr::Float_phys L1=(v-L2-L3-L4);
    // Term-by-term derivs
    const fmr::Float_phys L1r=-v, L2r=v , L3r=0., L4r=0.;
    const fmr::Float_phys L1s=-v, L2s=0., L3s=v , L4s=0.;
    const fmr::Float_phys L1t=-v, L2t=0., L3t=0., L4t=v ;
    // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
    //FMR_RESTRICT Mesh::vals g(3*10);
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
    const fmr::Float_phys L2=x[0]*v, L3=x[1]*v, L4=x[2]*v;
    const fmr::Float_phys L1=(v-L2-L3-L4);
    // Term-by-term derivs
    const fmr::Float_phys L1r=-v, L2r=v , L3r=0., L4r=0.;
    const fmr::Float_phys L1s=-v, L2s=0., L3s=v , L4s=0.;
    const fmr::Float_phys L1t=-v, L2t=0., L3t=0., L4t=v ;
    // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
    //FMR_RESTRICT Mesh::vals g(3*20);
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
  default: break;//const FMR_RESTRICT Mesh::vals g={};
  };
  return g;
}
#endif
//---------------------------- tet integration -------------------------------
// Tetrahedral integration points and weights
// for linear-shaped tetrahedra (edge nodes are interpolated),
// the Jacobian is constant and independent of the int pt locations.
// So, only ONE 3x3+1 (Jacobian+det) is needed for each element
// regardless of tet element order.
// Volume of natural tet is 1/6.
//
static constexpr fmr::Local_int tets_intg_1_n = 1;// Preferred P1
static constexpr
fmr::Phys_float tets_intg_1_ptwt [4* tets_intg_1_n] = {
  0.25,0.25,0.25, 1.0/6.0
};
static constexpr fmr::Local_int tets_intg_4_n = 4;// Preferred P2
static constexpr fmr::Phys_float a2 = 0.5854101966249685;
  // a2 = (5.0+3.0*std::sqrt(5.0))/20.0;
static constexpr fmr::Phys_float b2 = 0.1381966011250105;
  // b2 = (5.0-std::sqrt(5.0))/20.0;
static constexpr
fmr::Phys_float tets_intg_4_ptwt [4* tets_intg_4_n] = {
  b2,b2,b2, 0.25 / 6.0,
  a2,b2,b2, 0.25 / 6.0,
  b2,a2,b2, 0.25 / 6.0,
  b2,b2,a2, 0.25 / 6.0
};
static constexpr fmr::Local_int tets_intg_5_n = 5;// Alternate P2
//NOTE tet20s don't converge with 5-point rule
//NOTE Triple-checked these 5-point rule values
static constexpr
fmr::Phys_float tets_intg_5_ptwt [4* tets_intg_5_n] = {
  0.25   , 0.25   , 0.25   ,-4.0/ 30.0,
  0.5    , 1.0/6.0, 1.0/6.0, 9.0/120.0,
  1.0/6.0, 0.5    , 1.0/6.0, 9.0/120.0,
  1.0/6.0, 1.0/6.0, 0.5    , 9.0/120.0,
  1.0/6.0, 1.0/6.0, 1.0/6.0, 9.0/120.0
};
static constexpr fmr::Local_int tets_intg_10_n = 10;//Preferred B3
// From Lee Shunn, Frank Ham, Symmetric quadrature rules for tetrahedra
// based on a cubic close-packed lattice arrangement, 2012
static constexpr
fmr::Phys_float
  a0=0.0738349017262234    , a1=0.7784952948213300,
  b0=0.0937556561159491    , b1=0.4062443438840510,
  w0=0.0476331348432089/6.0, w1=0.1349112434378610/6.0;
static constexpr
fmr::Phys_float tets_intg_10_ptwt [4* tets_intg_10_n] = {
  a0,a0,a0, w0,
  a1,a0,a0, w0,
  a0,a1,a0, w0,
  a0,a0,a1, w0,
  b1,b0,b0, w1,
  b0,b1,b0, w1,
  b0,b0,b1, w1,
  b0,b1,b1, w1,
  b1,b0,b1, w1,
  b1,b1,b0, w1
};
static constexpr fmr::Local_int tets_intg_11_n = 11;//OLD B3
//NOTE This converges tet20 meshes
static constexpr fmr::Phys_float a3=0.3994035761667992;
// a3 = (1.+std::sqrt(5./14.))/4.;
static constexpr fmr::Phys_float b3=0.1005964238332008;
// b3 = (1.-std::sqrt(5./14.))/4.;
static constexpr
fmr::Phys_float tets_intg_11_ptwt [4* tets_intg_11_n] = {
   0.25    , 0.25    , 0.25    , -74.0/ 5625.0,
   1.0/14.0, 1.0/14.0, 1.0/14.0, 343.0/45000.0,
  11.0/14.0, 1.0/14.0, 1.0/14.0, 343.0/45000.0,
   1.0/14.0,11.0/14.0, 1.0/14.0, 343.0/45000.0,
   1.0/14.0, 1.0/14.0,11.0/14.0, 343.0/45000.0,
  b3,a3,a3, 56.0/2250.0,
  a3,b3,a3, 56.0/2250.0,
  a3,a3,b3, 56.0/2250.0,
  a3,b3,b3, 56.0/2250.0,
  b3,a3,b3, 56.0/2250.0,
  b3,b3,a3, 56.0/2250.0
};
//================================== brck ====================================
static constexpr fmr::Local_int brck_d      =  3;// spatial dimension
static constexpr fmr::Local_int brck_vert_n =  8;
static constexpr fmr::Local_int brck_edge_n = 12;
static constexpr fmr::Local_int brck_face_n =  6;
//
static constexpr
//TODO brick elem edge and face conventions
fmr::Local_int brck_vert_edge_bars [bars_vert_n * brck_edge_n]
  = {
  0,1, 1,2, 2,3, 3,0,
  4,5, 5,6, 6,7, 7,4,
  0,4, 1,5, 2,6, 3,7
};
static constexpr
fmr::Local_int brck_vert_face_quad [quad_vert_n * brck_face_n]
  = { 
  0,1,2,3,
  7,6,5,4,
  0,4,5,1,
  1,5,6,2,
  2,6,7,3,
  3,7,4,0
};
static constexpr fmr::Geom_float brck_meas       =  8.0;// natural elem volume
static constexpr fmr::Geom_float brck_face_meas  = 24.0;// elem surface area
static constexpr
fmr::Geom_float brck_vert_coor [brck_vert_n * brck_d ]// transposed coor_vert
  = {
                  // vertex   7---------6                //
  -1.0,-1.0,-1.0, // 0       /|        /|                //
   1.0,-1.0,-1.0, // 1      / |       / |                //
   1.0, 1.0,-1.0, // 2     /  |      /  |                //
  -1.0, 1.0,-1.0, // 3    4---------5   |                //
                  //      |   |     |   |                //
  -1.0,-1.0, 1.0, // 4    |   3-----|---2                //
   1.0,-1.0, 1.0, // 5    |  /      |  /      z  y       //
   1.0, 1.0, 1.0, // 6    | /       | /       | /        //
  -1.0, 1.0, 1.0  // 7    |/        |/        |/         //
};                //      0---------1         o--x       //
static constexpr
fmr::Geom_float brck_coor_vert [brck_d * brck_vert_n]// transposed vert_coor
  = {
  -1.0, 1.0, 1.0,-1.0, -1.0, 1.0, 1.0,-1.0,
  -1.0,-1.0, 1.0, 1.0, -1.0,-1.0, 1.0, 1.0,
  -1.0,-1.0,-1.0,-1.0,  1.0, 1.0, 1.0, 1.0
};
#if 0
  const Mesh::ints vert_conn={ 0,1,2,3, 4,5,6,7 };
  const Mesh::ints vert_edge// Bar2 connectivity
    ={ 0,1, 1,2, 2,3,3,0, 4,5,5,6,6,7,7,4, 0,4,1,5,2,6,3,7 };
  const Mesh::ints vert_face
    ={ 0,1,2,3, 7,6,5,4, 0,4,5,1, 2,3,7,6, 0,3,7,4, 1,5,6,2 };
  const Mesh::vals node_coor={
                   //       7---------6
     0.0, 0.0, 0.0,//      /|        /|
     1.0, 0.0, 0.0,//     / |       / |
     1.0, 1.0, 0.0,//    /  |      /  |
     0.0, 1.0, 0.0,//   4---------5   |
     0.0, 0.0, 1.0,//   |   |     |   |
     1.0, 0.0, 1.0,//   |   3-----|---2
     1.0, 1.0, 1.0,//   |  /      |  /
     0.0, 1.0, 1.0 //   | /       | /
                   //   |/        |/
  };               //   0---------1
#endif
//=========================== interface triangle =============================
// e.g., cohesive elements
static constexpr fmr::Local_int fac3_d      = 2;// spatial dimension
static constexpr fmr::Local_int fac3_vert_n = 6;
static constexpr fmr::Local_int fac3_edge_n = 6;
static constexpr fmr::Local_int fac3_face_n = 2;
static constexpr
  fmr::Local_int fac3_vert_conn [fac3_vert_n] = {0,1,2, 3,4,5};
//
static constexpr
fmr::Local_int fac3_vert_edge_bars [bars_vert_n * fac3_edge_n]
  = {
  0,1, 1,2, 2,0,
  3,4, 4,5, 5,3
};
static constexpr
fmr::Local_int fac3_vert_face_tris [tris_vert_n * fac3_face_n]
  = { 
  0,1,2, 5,4,3
};
static constexpr fmr::Geom_float fac3_meas  = 1.0;// ntrl elem surface area
static constexpr
fmr::Geom_float fac3_vert_coor [fac3_vert_n * fac3_d ]// transposed coor_vert
  = {
            // vertex             |   2-----1  -+                  //
  0.0, 0.0, // 0                  |    \   /    |                  //
  1.0, 0.0, // 1                  |     \ /     |   zero           //
  0.0, 1.0, // 2    5,2-----1,4   |      0      | thickness        //
            //         \   /      |             |                  //
  0.0, 0.0, // 3        \ /       |   5-----4  -+                  //
  1.0, 0.0, // 4         0,3      |    \   /        y z x          //
  0.0, 1.0  // 5                  |     \ /          \|/           //
};          //                    |      3            o            //
static constexpr
fmr::Geom_float fac3_coor_vert [fac3_d * fac3_vert_n]// transposed vert_coor
  = {
  0.0, 1.0, 0.0,  0.0, 1.0, 0.0,
  0.0, 0.0, 1.0,  0.0, 0.0, 1.0
};
//=========================== interface quadangles ===========================
// e.g., cohesive elements
static constexpr fmr::Local_int fac4_d      = 2;// spatial dimension
static constexpr fmr::Local_int fac4_vert_n = 8;
static constexpr fmr::Local_int fac4_edge_n = 8;
static constexpr fmr::Local_int fac4_face_n = 2;
static constexpr
  fmr::Local_int fac4_vert_conn [fac4_vert_n] = {0,1,2,3, 4,5,6,7};
//
static constexpr
fmr::Local_int fac4_vert_edge_bars [bars_vert_n * fac4_edge_n]
  = {
  0,1, 1,2, 2,3, 3,0,
  4,5, 5,6, 6,7, 7,4
};
static constexpr
fmr::Local_int fac4_vert_face_quad [quad_vert_n * fac4_face_n]
  = { 
  0,1,2,3, 7,6,5,4
};
static constexpr fmr::Geom_float fac4_meas  = 8.0;// natural element area
static constexpr
fmr::Geom_float fac4_vert_coor [fac4_vert_n * fac4_d ]// transposed coor_vert
  = {
             // vertex                     |       3---------2  -+           //
  -1.0,-1.0, // 0                          |      /         /    |           //
   1.0,-1.0, // 1                          |     /         /     |   zero    //
   1.0, 1.0, // 2        7,3---------2,6   |    /         /      | thickenss //
  -1.0, 1.0, // 3         /         /      |   0---------1       |           //
             //          /         /       |                     |           //
  -1.0,-1.0, // 4       /         /        |       7---------6  -+           //
   1.0,-1.0, // 5    4,0---------1,5       |      /         /        z  y    //
   1.0, 1.0, // 6                          |     /         /         | /     //
  -1.0, 1.0  // 7                          |    /         /          |/      //
};           //                            |   4---------5           o--x    //
static constexpr
fmr::Geom_float fac4_coor_vert [fac4_d * fac4_vert_n]// transposed vert_coor
  = {
  -1.0, 1.0, 1.0,-1.0,  -1.0, 1.0, 1.0,-1.0,
  -1.0,-1.0, 1.0, 1.0,  -1.0,-1.0, 1.0, 1.0
};

} } }//end femera::grid::fems:: namespace
