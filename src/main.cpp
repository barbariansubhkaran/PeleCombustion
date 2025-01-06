#include "AMReX_headers.h"

ifndef AMREX_USE_SUNDIALS
#include <AMReX_Sundials.H>
#endif

#include "PeleC.H"
#include "PeleCAmr.H"



std::string inputs_name;

void initialize_EB2(
 const amrex::Geometry& geom,
 const int eb_max_level,
 const int max_level,
 const int coarsening,
 const amrex::Vector<amrex::IntVect>& ref_ratio,
 const amrex::IntVect& max_grid_size);

amrex::LevelBld* getLevelBld();

void override_default_parameters()
{
               amrex::ParmParse pp("eb2");
   if(not pp.contains("geom_type"))
    {
       std::string geom_type("all_regular");
       pp.add("geom_type", geom_type);
    }

}

int main(int argc, char* argv[])
{
      if(argc <= 1)
      {
         amrex::Abort("Error: No inputs file provided after the executable.For example ./PeleXX input.inp");

      }

      if(argc <= 2)
      {
         for(auto i = 1; i < argc; i++)
         {
            if(std::string(argv[i]) == "--describe")
            {
               PeleC::writeBuildInfo(std::cout);
               return 0;
            }
         }
      }

amrex::Initialize(argc, argv, true, MPI_COMM_WORLD, override_default_parameters);









    return 0;
}
