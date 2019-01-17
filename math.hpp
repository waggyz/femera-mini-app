
inline FLOAT_MESH inner_product(
  RESTRICT const Mesh::vals &a,const RESTRICT Mesh::vals &b ){
  const size_t ii=a.size(); FLOAT_MESH s=0.0;
#pragma omp parallel for reduction(+:s)
  for(size_t i=0; i<ii; i++){
    s+=a[i]*b[i];
  };
  return s;
};
inline FLOAT_MESH inner_product(
  RESTRICT const Mesh::vals &a,const RESTRICT Mesh::vals &b, FLOAT_MESH s){
  const size_t ii=a.size();
#pragma omp parallel for reduction(+:s)
  for(size_t i=0; i<ii; i++){
    s+=a[i]*b[i];
  };
  return s;
};
inline RESTRICT Mesh::vals MatMulNx3x3T(
  RESTRICT const Mesh::vals& a,RESTRICT const Mesh::vals& b){
  const auto N=a.size();///3;
  RESTRICT Mesh::vals c(a.size());//={0.0};
  for(size_t i=0; i<N; i+=3){// b is transposed
    //for(size_t k=0; k<3; k++){//FIXED Unroll this
      c[i+ 0]=inner_product(
        a[std::slice(i, 3, 1)],b[std::slice(3*0, 3, 1)] );
      c[i+ 1]=inner_product(
        a[std::slice(i, 3, 1)],b[std::slice(3*1, 3, 1)] );
      c[i+ 2]=inner_product(
        a[std::slice(i, 3, 1)],b[std::slice(3*2, 3, 1)] );
    //};
  };
  return c;
  };
inline RESTRICT Mesh::vals MatMul3x3xN(
  RESTRICT const Mesh::vals& a,RESTRICT const Mesh::vals& b){
  const auto N=b.size()/3;
  RESTRICT Mesh::vals c(b.size());//={0.0};
  for(size_t k=0; k<N; k++){// b is NOT transposed
    //for(size_t i=0; i<3; i++){//FIXED Unroll this
      c[N*0+ k]=inner_product(
        a[std::slice(3*0, 3, 1)],b[std::slice(k, 3, N)] );
      c[N*1+ k]=inner_product(
        a[std::slice(3*1, 3, 1)],b[std::slice(k, 3, N)] );
      c[N*2+ k]=inner_product(
        a[std::slice(3*2, 3, 1)],b[std::slice(k, 3, N)] );
    //};
  };
  return c;
  };//end MatMul3x3xN()
inline RESTRICT Mesh::vals MatMul2x2xN(
  RESTRICT const Mesh::vals& a,RESTRICT const Mesh::vals& b){
  const auto N=b.size()/2;
  RESTRICT Mesh::vals c(b.size());//={0.0};
  for(size_t k=0; k<N; k++){// b is NOT transposed
    //for(size_t i=0; i<2; i++){//FIXED Unroll this
      c[N*0+ k]=inner_product(
        a[std::slice(2*0, 2, 1)],b[std::slice(k, 2, N)] );
      c[N*1+ k]=inner_product(
        a[std::slice(2*1, 2, 1)],b[std::slice(k, 2, N)] );
    //};
  };
  return c;
  };//end MatMul2x2xN()
inline RESTRICT Mesh::vals MatMul3x3xNT(
  RESTRICT const Mesh::vals& a,RESTRICT const Mesh::vals& b){
  const auto N=b.size()/3;
  RESTRICT Mesh::vals c(b.size());//={0.0};
  for(size_t k=0; k<N; k++){// b is transposed
    //for(size_t i=0; i<3; i++){//FIXED Unroll this
      c[3*0+ k]=inner_product(
        a[std::slice(3*0, 3, 1)],b[std::slice(3*k, 3, 1)] );
      c[3*1+ k]=inner_product(
        a[std::slice(3*1, 3, 1)],b[std::slice(3*k, 3, 1)] );
      c[3*2+ k]=inner_product(
        a[std::slice(3*2, 3, 1)],b[std::slice(3*k, 3, 1)] );
    //};
  };
  return c;
  };//end MatMul3x3xN()
inline RESTRICT Mesh::vals MatMul2x2xNT(
  RESTRICT const Mesh::vals& a,RESTRICT const Mesh::vals& b){
  const auto N=b.size()/2;
  RESTRICT Mesh::vals c(b.size());//={0.0};
  for(size_t k=0; k<N; k++){// b is transposed
    //for(size_t i=0; i<3; i++){//FIXED Unroll this
      c[2*0+ k]=inner_product(
        a[std::slice(2*0, 2, 1)],b[std::slice(2*k, 2, 1)] );
      c[2*1+ k]=inner_product(
        a[std::slice(2*1, 2, 1)],b[std::slice(2*k, 2, 1)] );
    //};
  };
  return c;
  };//end MatMul2x2xNT()
inline RESTRICT Mesh::vals MatMul3xNx3T(
  RESTRICT const Mesh::vals& a,RESTRICT const Mesh::vals& b){
  const auto N=a.size()/3;
  RESTRICT Mesh::vals c(9);//={0.0};
  for(size_t i=0; i<3; i++){// b is transposed
    c[3*i   ]=inner_product( a[std::slice(N*i, N, 1)],
                             b[std::slice(0  , N, 1)] );
    c[3*i+ 1]=inner_product( a[std::slice(N*i, N, 1)],
                             b[std::slice(N  , N, 1)] );
    c[3*i+ 2]=inner_product( a[std::slice(N*i, N, 1)],
                             b[std::slice(N*2, N, 1)] );
    };
  return c;
  };//end MatMul3xNx3()
inline RESTRICT Mesh::vals MatMul2xNx2T(
  RESTRICT const Mesh::vals& a,RESTRICT const Mesh::vals& b){//, const int N){
  const auto N=a.size()/2;
  RESTRICT Mesh::vals c(4);//={0.0};
  for(size_t i=0; i<2; i++){// b is transposed
    c[2*i   ]=inner_product( a[std::slice(N*i, N, 1)],
                             b[std::slice(0  , N, 1)] );
    c[2*i+ 1]=inner_product( a[std::slice(N*i, N, 1)],
                             b[std::slice(N  , N, 1)] );
    };
  return c;
  };//end MatMul2xNx2()
inline RESTRICT Mesh::vals MatMul2xNx2(
  RESTRICT const Mesh::vals& a,RESTRICT const Mesh::vals& b){//, const int N){
  const auto N=a.size()/2;
  RESTRICT Mesh::vals c(4);//={0.0};
  for(size_t i=0; i<2; i++){// b is transposed
    c[2*i   ]=inner_product( a[std::slice(N*i, N, 1)],
                             b[std::slice(0  , N, 2)] );
    c[2*i+ 1]=inner_product( a[std::slice(N*i, N, 1)],
                             b[std::slice(1  , N, 2)] );
    };
  return c;
  };//end MatMul2xNx2()
#if (FLOAT_MESH!=FLOAT_PHYS)
inline FLOAT_PHYS inner_product(
  RESTRICT const Phys::vals &a,const RESTRICT Phys::vals &b ){
  const size_t ii=a.size(); FLOAT_PHYS s=0.0;
  for(size_t i=0; i<ii; i++){ s+=a[i]*b[i]; };
  return s;
};
inline FLOAT_PHYS inner_product(
  RESTRICT const Phys::vals &a,const RESTRICT Phys::vals &b, FLOAT_PHYS s){
  const size_t ii=a.size();
  for(size_t i=0; i<ii; i++){ s+=a[i]*b[i]; };
  return s;
};
inline RESTRICT Phys::vals MatMul2x2xN(
  RESTRICT const Phys::vals &a,RESTRICT const Phys::vals &b){
  const auto N=b.size()/2;
  RESTRICT Phys::vals c(b.size());//={0.0};
  for(size_t k=0; k<N; k++){// b is NOT transposed
    //for(size_t i=0; i<2; i++){//FIXED Unroll this
      c[N*0+ k]=inner_product(
        a[std::slice(2*0, 2, 1)],b[std::slice(k, 2, N)] );
      c[N*1+ k]=inner_product(
        a[std::slice(2*1, 2, 1)],b[std::slice(k, 2, N)] );
    //};
  };
  return c;
  };
inline RESTRICT Phys::vals MatMul3x3xN(
  RESTRICT const Phys::vals& a,RESTRICT const Phys::vals& b){
  const auto N=b.size()/3;
  RESTRICT Phys::vals c(b.size());//={0.0};
  for(size_t k=0; k<N; k++){// b is NOT transposed
    //for(size_t i=0; i<3; i++){//FIXED Unroll this
      c[N*0+ k]=inner_product(
        a[std::slice(3*0, 3, 1)],b[std::slice(k, 3, N)] );
      c[N*1+ k]=inner_product(
        a[std::slice(3*1, 3, 1)],b[std::slice(k, 3, N)] );
      c[N*2+ k]=inner_product(
        a[std::slice(3*2, 3, 1)],b[std::slice(k, 3, N)] );
    //};
  };
  return c;
  };//end MatMul3x3xN()
inline RESTRICT Phys::vals MatMul2x2xN(
    RESTRICT const Phys::vals& a,RESTRICT const Phys::vals& b){
    const auto N=b.size()/2;
    RESTRICT Phys::vals c(b.size());//={0.0};
    for(size_t k=0; k<N; k++){// b is NOT transposed
      //for(size_t i=0; i<2; i++){//FIXED Unroll this
        c[N*0+ k]=inner_product(
          a[std::slice(2*0, 2, 1)],b[std::slice(k, 2, N)] );
        c[N*1+ k]=inner_product(
          a[std::slice(2*1, 2, 1)],b[std::slice(k, 2, N)] );
      //};
    };
    return c;
  };//end MatMul2x2xN()
  inline RESTRICT Phys::vals MatMul3xNx3T(
    RESTRICT const Phys::vals& a,RESTRICT const Phys::vals& b){
    const auto N=a.size()/3;
    RESTRICT Phys::vals c(9);//={0.0};
    for(size_t i=0; i<3; i++){// b is transposed
      c[3*i   ]=inner_product( a[std::slice(N*i, N, 1)],b[std::slice(0  , N, 1)] );
      c[3*i+ 1]=inner_product( a[std::slice(N*i, N, 1)],b[std::slice(N  , N, 1)] );
      c[3*i+ 2]=inner_product( a[std::slice(N*i, N, 1)],b[std::slice(N*2, N, 1)] );
    };
    return c;
  };//end MatMul3xNx3()
inline RESTRICT Phys::vals MatMul2xNx2T(
    RESTRICT const Phys::vals& a,RESTRICT const Phys::vals& b){//, const int N){
    const auto N=a.size()/2;
    RESTRICT Phys::vals c(4);//={0.0};
    for(size_t i=0; i<2; i++){// b is transposed
      c[2*i   ]=inner_product( a[std::slice(N*i, N, 1)],b[std::slice(0  , N, 1)] );
      c[2*i+ 1]=inner_product( a[std::slice(N*i, N, 1)],b[std::slice(N  , N, 1)] );
    };
    return c;
  };//end MatMul2xNx2()
#endif

