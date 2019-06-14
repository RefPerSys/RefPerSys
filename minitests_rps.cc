/****************************************************************
 * file minitests_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *  Mini-tests, temporarily before the primordial persistence
 *  milestone.  See https://gitlab.com/bstarynk/refpersys/milestones/1
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      <https://refpersys.gitlab.io>
 *
 * License:
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/


#include "refpersys.hh"

void
Rps_ObjectRef::tiny_benchmark_1(Rps_CallFrameZone*callingfra, unsigned num)
{
  RPS_LOCALFRAME(callingfra, /*descr:*/nullptr,
                 Rps_SetValue setv;
                );
  _.setv = Rps_SetValue::tiny_benchmark_1(RPS_CURFRAME, num);
  assert (_.setv);
  RPS_FATAL("unimplemented Rps_ObjectRef::tiny_benchmark_1 num=%u", num);
#warning Rps_ObjectRef::tiny_benchmark_1 unimplemented
} // end of Rps_ObjectRef::tiny_benchmark_1



Rps_SetValue
Rps_SetValue::tiny_benchmark_1(Rps_CallFrameZone*callingfra, unsigned num)
{
  constexpr unsigned benchsize = Rps_ObjectZone::at_sorted_thresh+5;
  RPS_LOCALFRAME(callingfra, /*descr:*/nullptr,
                 Rps_SetValue setresv;
                 Rps_Value arrval[benchsize];
                 Rps_ObjectRef arrob[benchsize];
                 union
  {
    Rps_SetValue setv;
    Rps_TupleValue tupv;
  };
  Rps_QuasiObjectVector* qvecob;
                );
  RPS_ASSERT (num > 10);
  RPS_ASSERTPRINTF(num > 10, "num=%u", num);
  double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0, t6 = 0.0;
  {
    char msgbuf[64];
    memset(msgbuf, 0, sizeof(msgbuf));
    snprintf(msgbuf, sizeof(msgbuf), "starting tiny_benchmark_1 num#%u",
             num);
    Rps_BackTrace::run_full_backtrace(2, msgbuf);
    fflush(nullptr);
  };
  t0 = rps_thread_cpu_time();
  assert (t0 > 0.0);
  for (auto i = 0u; i<benchsize; i++)
    {
      _.arrval[i] = Rps_Value(Rps_ObjectRef::make(RPS_CURFRAME));
      _.arrob[i] = Rps_ObjectRef::make(RPS_CURFRAME);
      if (i == benchsize/2)
        t1 = rps_thread_cpu_time();
    }
  assert (benchsize < 0xff);
  t2 = rps_thread_cpu_time();
  std::cout << "tiny_benchmark_1 (" << __FILE__ << ":" << __LINE__
            << ") made " << (2*benchsize) << " objects in "
            << (t2-t0) << " thread-CPU seconds, half of them in "
            << (t1-t0)  << " thread-CPU seconds, so "
            << 1.0e6*(t2-t0)/(2*benchsize) << " µsec/obj." << std::endl;
  /// think of this tiny_benchmark_1 as some "bytecode" machine
  /// executing a random stream of bytecodes.
  for (unsigned cnt=0; cnt<num; cnt++)
    {
      auto rop = Rps_Random::random_quickly_4bits();
      assert (rop >= 0 && rop < 16);
      switch(rop)
        {
        case 0:
          // create a random new object
          _.arrval[Rps_Random::random_quickly_8bits() % benchsize]
            = Rps_Value(Rps_ObjectRef::make(RPS_CURFRAME));
          break;
        case 1:
        case 2:
          // fetch an existing object as random value
          _.arrval[Rps_Random::random_quickly_8bits() % benchsize]
            = Rps_Value(_.arrob[Rps_Random::random_quickly_8bits()
                                % benchsize]);
          break;
        case 3:
        case 4:
          // create a random small tagged int
          _.arrval[Rps_Random::random_quickly_8bits() % benchsize]
            = Rps_IntValue(Rps_Random::random_quickly_8bits() - 100);
          break;
        case 5:
        case 6:
          // create a random string value
        {
          Rps_Id randid (nullptr);
          char cbuf[24];
          memset (cbuf, 0, sizeof(cbuf));
          randid.to_cbuf24(cbuf);
          cbuf[0] = ' ' + (Rps_Random::random_quickly_8bits() % 64 + 1);
          cbuf[Rps_Random::random_quickly_4bits() + 4] = (char)0;
          _.arrval[Rps_Random::random_quickly_8bits() % benchsize]
            = Rps_StringValue(RPS_CURFRAME,cbuf);
          break;
        }
        case 7:
        case 8:
          // put a random attribute
        {
          auto& destob =
            _.arrob[Rps_Random::random_quickly_8bits() % benchsize];
          if (destob)
            destob->put_attr(RPS_CURFRAME,
                             _.arrob[Rps_Random::random_quickly_8bits() % benchsize],
                             _.arrval[Rps_Random::random_quickly_8bits() % benchsize]);
          break;
        }
        case 9:
        case 10:
          // append a random component
        {
          auto& destob =
            _.arrob[Rps_Random::random_quickly_8bits() % benchsize];
          if (destob)
            destob->append_component
            (RPS_CURFRAME,
             _.arrval[Rps_Random::random_quickly_8bits() % benchsize]);
          break;
        }
        case 11:
          // if possible, swap a random _.arrob entry with a random
          // _.arrval entry (if it holds an object)
        {
          auto& destob =
            _.arrob[Rps_Random::random_quickly_8bits() % benchsize];
          auto& othval =
            _.arrval[Rps_Random::random_quickly_8bits() % benchsize];
          if (destob && othval && othval.as_object() != nullptr)
            {
              Rps_ObjectRef rdestob = destob;
              Rps_Value vothval = othval;
              destob = vothval.as_object();
              othval =  Rps_Value(rdestob);
            };
        }
        break;
        case 12:
          // create some random set
        {
          auto &destval =
            _.arrval[Rps_Random::random_quickly_8bits() % benchsize];
          unsigned rsiz =
            2+ Rps_Random::random_quickly_8bits() % (benchsize/3);
          _.qvecob =
            Rps_QuasiObjectVector::make_cleared(RPS_CURFRAME, rsiz);
#warning Rps_SetValue::tiny_benchmark_1 should create some random set
        }
        break;
        case 13:
          // create some random tuple
        {
          auto &destval =
            _.arrval[Rps_Random::random_quickly_8bits() % benchsize];
          unsigned rsiz =
            2+ Rps_Random::random_quickly_8bits() % (benchsize/3);
          _.qvecob =
            Rps_QuasiObjectVector::make_cleared(RPS_CURFRAME, rsiz);
#warning Rps_SetValue::tiny_benchmark_1 should create some random tuple
        }
        break;
        case 14:
        // maybe remove an attribute
        case 15:
          // maybe remove a component
          break;
        }
    }
} // end Rps_SetValue::tiny_benchmark_1


/// end of file minitests_rps.cc
