int main(){
  int a=-6;
#pragma omp simd
  for(int i=0;i<4; i++){ a+=i; };
  return a;
}
