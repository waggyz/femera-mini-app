#include "gtest/gtest.h"
#include "gmock/gmock.h"      // ::testing::HasSubstr

#include "../base.h"          // test fmr::data:: functions
//#include "../Main/Plug.hpp"

TEST( DataCGNS, IsLoaded ){
  EXPECT_THAT( fmr::detail::main->data->print_summary (),
    ::testing::HasSubstr ("CGNS") );
}
#if 0
TEST( DataCGNS, FindModelInFile ){
  EXPECT_EQ( 0, fmr::data::get_models.size () );
  EXPECT_EQ( 1, fmr::data::add_file (
    std::string(FMR_BUILD_DIR"/Data/does_not_exist.cgns")) );
  EXPECT_EQ( 0, fmr::data::add_file (
    std::string(FMR_BUILD_DIR"/Data/cube-tet6p1n1.cgns")) );
  EXPECT_EQ( 1, fmr::data::get_sims_n () );
  EXPECT_EQ( std::string(FMR_BUILD_DIR"/Data/cube-tet6p1n1.cgns"),
   fmr::data::get_file_name () );
  EXPECT_EQ( "cube-tet6p1n1.cgns"), fmr::data::get_model_name (0) );
}
#endif

int main (int argc, char** argv){
  return fmr::exit (fmr::init (&argc,argv));
}

#if 0
  EXPECT_EQ("", fmr::detail::main->data->print_details () );
TEST( Dcgn, SizeofDcgnGE448 ){
  EXPECT_GE( sizeof(Femera::Dcgn), 448 );
}
TEST( Dcgn, TrivialTest ){ EXPECT_EQ( 2+2, 4 );}
#endif

#if 0
using namespace Femera;

Proc* tst_proc = new Proc();
Dcgn* empty = new Dcgn (tst_proc);
Dcgn* gmsh  = new Dcgn (tst_proc);

TEST( NewCGNS, CheckDefaultConfig ){
  EXPECT_EQ( 0                     , empty->init( MPI_COMM_WORLD )  );
  EXPECT_EQ( empty->standard       , Data::Standard::CGNS           );
  EXPECT_EQ( empty->get_file_name(), std::string("new_femera_1.cgn"));
  EXPECT_EQ( empty->file_exts[0]   , std::string("cgn")             );
  EXPECT_EQ( empty->precision      , Data::Precision::FLOAT_DOUBLE  );
  EXPECT_EQ( empty->require_access , Data::Access::CHECK            );
  EXPECT_EQ( empty->current_access , Data::Access::CHECK            );
  EXPECT_EQ( empty->format         , fmr::data::detail::File_format::DCGN_HDF5);
}
TEST( NewCGNS, CreateOK ){
  EXPECT_EQ( 0, empty->Data::open(Data::Access::NEW,
    FMR_BUILD_DIR"/Data/empty.cgn") );
}
TEST( NewCGNS, CheckFilename ){
  EXPECT_EQ( empty->get_file_name(),
    std::string(FMR_BUILD_DIR"/Data/empty.cgn"));
}
TEST( NewCGNS, CheckConfig ){
//  EXPECT_GE( empty->file_version_cgns  , 5.0f );
  EXPECT_EQ( empty->format        , fmr::data::detail::File_format::DCGN_HDF5 );
  //EXPECT_EQ( empty->precision     , Data::Precision::FLOAT_DOUBLE );
}
#if 1
TEST( NewCGNS, CheckBaseName ){
  EXPECT_EQ( empty->get_model_names().size(), std::size_t( 1 ));
  EXPECT_EQ( empty->get_model_names().front(), std::string("new_model_1") );
}
#endif
TEST( NewCGNS, CloseOK ){
  EXPECT_EQ( 0, empty->close() );
}
//TEST( NewCGNS, CheckIsCGNS ){
//  EXPECT_EQ( true, empty->is_this_type() );
//}

TEST( GmshCGNS, OpenOK ){
  EXPECT_EQ( 0, gmsh->init( MPI_COMM_WORLD ) );
  EXPECT_EQ( 0, gmsh->Data::open(Data::Access::READ,
    std::string(FMR_BUILD_DIR"/Data/cube-tet6p1n1.cgns")) );
}
TEST( GmshCGNS, CheckConfig ){
//  EXPECT_GE( gmsh->file_version_cgns  , 5.0f );
  EXPECT_EQ( gmsh->format        , fmr::data::detail::File_format::DCGN_HDF5 );
  EXPECT_EQ( gmsh->precision     , Data::Precision::FLOAT_DOUBLE );
}
#if 1
TEST( GmshCGNS, CheckBasename ){
  EXPECT_EQ( gmsh->get_model_names().size() , std::size_t( 1 ));
  EXPECT_EQ( gmsh->get_model_names().front(), std::string("cube-tet6p1n1.cgns") );
}
#endif


#if 0
TEST( GmshCGNS, CheckBaseDesc ){
  EXPECT_EQ( gmsh->get_bas1_name_list().size(),std::size_t( 1 ));
  EXPECT_EQ( gmsh->get_bas1_name_list().front(), std::string("About") );
  EXPECT_EQ( gmsh->get_bas1_desc_list().size(),std::size_t( 1 ));
  EXPECT_EQ( gmsh->get_bas1_desc_list().front(), std::string("Created by gmsh") );
}
#endif
#if 0
TEST( gmshCGN, CheckZone ){ );
}
#endif
TEST( GmshCGNS, CloseOK ){
  EXPECT_EQ( 0, gmsh->close() );
}
int main(int argc, char** argv ){
  int err= tst_proc->init( &argc,argv );
  err= gmsh->exit( err );
  err= empty->exit( err );
  err= tst_proc->exit( err );// gtest run_all_tests is done here.
  delete tst_proc;
  return err;
}
#endif
