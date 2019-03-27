#include "types.h"


// initialise static members of Rps_Value_Data_Mostly_Copying
thread_local mps_arena_t Rps_Value_Data_Mostly_Copying::_arena = nullptr;
thread_local mps_pool_t Rps_Value_Data_Mostly_Copying::_pool = nullptr;
thread_local mps_ap_t Rps_Value_Data_Mostly_Copying::_allocpt = nullptr;


// initialise MPS arena
void Rps_Value_Data_Mostly_Copying::init_arena(void)
{
    const size_t arenasize = 32ul * 1024 * 1024;
    mps_arg_s args[1];
    mps_res_t res;

    MPS_ARGS_BEGIN(args)
    {
      MPS_ARGS_ADD(args, MPS_KEY_ARENA_SIZE, arenasize);
      res = mps_arena_create_k(&Rps_Value_Data_Mostly_Copying::_arena,
                               mps_arena_class_vm(), args);
    }
    MPS_ARGS_END(args);

    if (res != MPS_RES_OK)
      {
        perror("Couldn't create arena");
        abort();
      }
}



// initialise MPS area, pool, and allocation pointer
void Rps_Value_Data_Mostly_Copying::init_mps(void)
{
  Rps_Value_Data_Mostly_Copying::init_arena();
}

