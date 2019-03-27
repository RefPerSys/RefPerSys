#include "types.h"


// declare static members of Rps_Value_Data_Mostly_Copying
thread_local mps_arena_t Rps_Value_Data_Mostly_Copying::_arena = nullptr;
thread_local mps_pool_t Rps_Value_Data_Mostly_Copying::_pool = nullptr;
thread_local mps_ap_t Rps_Value_Data_Mostly_Copying::_allocpt = nullptr;


// initialize MPS arena
// the arena is requested from the OS
void 
Rps_Value_Data_Mostly_Copying::init_arena(void)
{
  // we may have to tune that `arenasize` a bit. Basile's guess is
  // that it should be related to the size of the L3 cache, and then
  // that number might be slightly too large. But that should be done
  // much later, when we do have some code actively using the MPS GC.
  const size_t arenasize = 32ul * 1024 * 1024; // 32 Megabytes
  // TODO: explain what happens when the 32 megabytes are exhausted?
  // is a new "arena" or "pool" or some 32 megabytes memory zone
  // created? How? When?
  mps_arg_s args[1];
  mps_res_t res;

  MPS_ARGS_BEGIN(args)
  {
    MPS_ARGS_ADD(args, MPS_KEY_ARENA_SIZE, arenasize);
    res = mps_arena_create_k(&Rps_Value_Data_Mostly_Copying::_arena,
                             mps_arena_class_vm(),
                             args);
  }
  MPS_ARGS_END(args);

  if (rps_unlikely(res != MPS_RES_OK))
    {
      perror("Couldn't create arena");
      abort();
    }

  // make sure we can pick up finalisation messages
  // TODO: need to check if it's OK to call here at this point
  mps_message_type_enable(Rps_Value_Data_Mostly_Copying::_arena,
                          mps_message_type_finalization());
}

// initialise MPS pool as Automatic Mostly Copying Type
// TODO: need Dr. Basile's advice on object format and object chains; for the
// time being, mps_pool_create_k() is being passed mps_args_none, but probably
// need need to be passed object format and object chain parameters
void 
Rps_Value_Data_Mostly_Copying::init_pool(void)
{
  // Basile thinks: if that short routine is called *only* from
  // Rps_Value_Data_Mostly_Copying::init_mps we could move its code
  // there.
  
  // TODO: do we need to create object format? Basile don't know yet. That could be decided later.
  // TODO: do we need to create object chain? Basile don't know yet. That could be decided later.
  // Basile reminds: the priority is the primordial persistence milestone.

  mps_res_t res = mps_pool_create_k(&Rps_Value_Data_Mostly_Copying::_pool,
                                    Rps_Value_Data_Mostly_Copying::_arena,
                                    mps_class_amc(),
                                    mps_args_none);

  if (rps_unlikely(res != MPS_RES_OK))
    {
      perror("Couldn't create AMC pool");
      abort();
    }
}


// initialise MPS allocation point
// the allocation point allows fast inline allocation for objects from the MPS
// pool
void 
Rps_Value_Data_Mostly_Copying::init_ap(void)
{
  mps_res_t res = mps_ap_create_k(&Rps_Value_Data_Mostly_Copying::_allocpt,
                                  Rps_Value_Data_Mostly_Copying::_pool,
                                  mps_args_none);

  if (rps_unlikely(res != MPS_RES_OK))
    {
      perror("Couldn't create allocation point");
      abort();
    }
}




// initialise MPS area, pool, and allocation pointer
void 
Rps_Value_Data_Mostly_Copying::init_mps(void)
{
  // Basile thinks: If these routines are only called here, we might want to move
  // their code inside. IMHO a 80 lines routine stays readable, but
  // needs a few (perhaps 3 or 4) lines of explanation.
  Rps_Value_Data_Mostly_Copying::init_arena();
  Rps_Value_Data_Mostly_Copying::init_pool();
  Rps_Value_Data_Mostly_Copying::init_ap();
} /*end Rps_Value_Data_Mostly_Copying::init_mps*/

