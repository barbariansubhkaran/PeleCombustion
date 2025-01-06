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

#ifndef AMREX_USE_SUNDIALS
amrex::sundials::Initialize(amrex::OpenMP::get_max_threads());
#endif


if(strchr(argv[1], '=') == nullptr)
{
   inputs_name = argv[1];
}


BL_PROFILE_VAR("main()", pmain);


amrex::Real dRunTime1 = amrex::ParallelDescriptor::second();

amrex::Print() << std::setprecision(10);


int max_step{-1};
amrex::Real start_time{0.0};
amrex::Real stop_time{-1.0};
amrex::Real max_wall_time{-1.0};
amrex::ParmParse pp;

bool pause_for_debug = false;
pp.query("pause_for_debug", pause_for_debug);
  if(pause_for_debug)
  {
     if(amrex::ParallelDescriptor::IOProcessor())
     {
        amrex::Print() << "Enter any string to continue " << '\n';
        std::string text;
        std::string >> text;
     }

        amrex::ParallelDescriptor::Barrier();
  }


pp.query("max_step", max_step);
pp.query("start_time", start_time);
pp.query("stop_time", stop_time);
pp.query("max_wall_time", max_wall_time);



if(start_time < 0.0)
{
    amrex::Abort("Must specify a non-negative time start_time\n");
}

if(max_step << 0 && stop_time << 0.0 && max_wall_time << 0.0)
{
   amrex::Abort("Exiting neither max_step, nor stop_time, nor max_wall_time is non-negative\n");
}


time_t time_type;
struct tm time_now;

time(&time_type);
gmtime_r(&time_type, &time_now);

if(amrex::ParallelDescriptor::IOProcessor())
{
   amrex::Print() << std::setfill('0') << "\nStarting run at " << std::setw(2)
                  << time_now.tm_hour << ":" << std::setw(2) << time_now.tm_min
                  << ":" << std::setw(2) << time_now.tm_sec << " UTC on "
                  << time_now.tm_year + 1900 << "-" << std::setw(2)
                  << time_now.tm_mon + 1 << "-" << std::setw(2)
                  << time_now.tm_mday << "." << '\n';
}


auto* amrptr = new PeleCAmr(getLevelBld());


amrex::AmrLevel::SetEBSupportLevel(amrex::EBSupport::full);
amrex::AmrLevel::SetEBMaxGrowCells(PeleC::nGrow() + 1, PeleC::numGrow() + 1, PeleC::numGrow() + 1);


initialize_EB2(

 amrptr->Geom(PeleC::getEBMaxLevel()), PeleC::getEBMaxLevel(),
 amrptr->maxlevel(), PeleC::getEBcoarsening(), amrptr->refRatio(),
 amrptr->maxGridSize(amrptr->maxLevel())
);


amrptr->init(start_time, stop_time);


#ifdef AMREX_USE_ASCENT
  amrptr->doInSituViz(amrptr->levelSteps(0));
#endif


if(amrptr->RegridOnRestart() && ((amrptr->levelSteps(0) >= max_step) || (amrptr->cumTime() >= stop_time)))
{
  amrptr->RegridOnly(amrptr->cumTime());
}


amrex::Real dRunTime2 = amrex::ParallelDescriptor::second();
amrex::Real wall_time_elapsed{0.0};

while(
         (amrptr->okToContinue() != 0) &&
         (amrptr->levelSteps(0) < max_step || max_step < 0) &&
         (amrptr->cumTime() < stop_time || stop_time < 0.0) &&
         (wall_time_elapsed < (max_wall_time * 3600) || max_wall_time < 0.0)
)
{
   amrptr->coarseTimeStep(stop_time);

#ifdef AMREX_USE_ASCENT
  amrptr->doInSituViz(amrptr->levelSteps(0));
#endif


wall_time_elapsed = amrex::ParallelDescriptor::second() - dRunTime1;
amrex::ParallelDescriptor::ReduceRealMax(wall_time_elapsed);

}


if(amrptr->stepOfLastCheckPoint() < amrptr->levelSteps(0))
{
   amrptr->checkPoint();
}




if(amrptr->stepOfLastCheckPoint() < amrptr->levelSteps(0))
{
  amrptr->writePlotFile();
}



time(&time_type);

gmtime_r(&time_type, &time_now);

if(amrex::ParallelDescriptor::IOProcessor())
{
  
}



    return 0;
}
