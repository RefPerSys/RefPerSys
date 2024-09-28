/****************************************************************
 * file machlearn_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to machine learning
 *
 * Debian packages: mlpack-bin mlpack-doc libmlpack-dev libensmallen-dev
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2024 - 2024 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * See websites: ensmallen.org and mlpack.org
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

// Define these to print extra informational output and warnings.
#define MLPACK_PRINT_INFO
#define MLPACK_PRINT_WARN

#include "mlpack.hpp"

//@@PKGCONFIG mlpack

extern "C" const char rps_machlearn_gitid[];
const char rps_machlearn_gitid[]= RPS_GITID;

extern "C" const char rps_machlearn_date[];
const char rps_machlearn_date[]= __DATE__;

extern "C" const char rps_machlearn_shortgitid[];
const char rps_machlearn_shortgitid[]= RPS_SHORTGITID;

/// we require mlpack.org version 4.5 since 4.4 won't build
/// see freelists.org/post/mlpack-git/MLPACK-version-checking-in-preprocessing,1
#if MLPACK_VERSION_MAJOR != 4 || MLPACK_VERSION_MINOR != 5
#error mlpack.org version 4.5 is required
#endif /* not mlpack 4.5 */

extern "C" rpsldpysig_t rpsldpy_machlearn;
class Rps_PayloadMachLearn;

extern "C" std::string rps_mlpack_get_version(void);

std::string rps_mlpack_get_version(void)
{
  return mlpack::util::GetVersion();
} // end rps_mlpack_get_version

class Rps_PayloadMachLearn : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend rpsldpysig_t rpsldpy_machlearn;
  friend Rps_PayloadMachLearn*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadMachLearn,Rps_ObjectZone*>(Rps_ObjectZone*);
  Rps_PayloadMachLearn(Rps_ObjectZone*owner);
  Rps_PayloadMachLearn(Rps_ObjectRef obr) :
    Rps_PayloadMachLearn(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadMachLearn();
  ///TODO: figure out how to use these....
  arma::mat _machlearn_matrix;
  arma::Row<size_t> _machlearn_labels;
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this)+sizeof(void*)-1) / sizeof(void*);
  };
public:
  static Rps_ObjectRef make_machlearn_object(Rps_CallFrame*callframe, Rps_ObjectRef obclass=nullptr, Rps_ObjectRef obspace=nullptr);
  virtual const std::string payload_type_name(void) const
  {
    return "machlearn";
  };
  inline Rps_PayloadMachLearn(Rps_ObjectZone*obz, Rps_Loader*ld);
  std::string matrix_file_path(void) const;
  std::string labels_file_path(void) const;
};        // end Rps_PayloadMachLearn

std::string
Rps_PayloadMachLearn::matrix_file_path(void) const
{
  std::string matrixpath;
  char buf[80];
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%s_machlearn_mat.csv",
           owner()->string_oid().c_str());
  matrixpath.assign(buf);
  return matrixpath;
} // end Rps_PayloadMachLearn::matrix_file_path

std::string
Rps_PayloadMachLearn::labels_file_path(void) const
{
  std::string labelspath;
  char buf[80];
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%s_machlearn_lab.csv",
           owner()->string_oid().c_str());
  labelspath.assign(buf);
  return labelspath;
} // end Rps_PayloadMachLearn::matrix_file_path


Rps_PayloadMachLearn::~Rps_PayloadMachLearn()
{
#warning incomplete Rps_PayloadMachLearn destructor
} // end Rps_PayloadMachLearn::~Rps_PayloadMachLearn

Rps_PayloadMachLearn::Rps_PayloadMachLearn(Rps_ObjectZone*ob)
  : Rps_Payload(Rps_Type::PaylMachlearn,ob)
{
} // end
void
Rps_PayloadMachLearn::gc_mark([[maybe_unused]] Rps_GarbageCollector& gc) const
{
#warning incomplete Rps_PayloadMachLearn::gc_mark
} // end Rps_PayloadMachLearn::gc_mark


void
Rps_PayloadMachLearn::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  // the payload contains no RefPerSys values so the scan is empty
  return;
} // end Rps_PayloadMachLearn::dump_scan

void
Rps_PayloadMachLearn::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_machlearn
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  std::string matrixpath = matrix_file_path();
  if (!access(matrixpath.c_str(), F_OK))
    {
      std::string backupmatrixpath = matrixpath+"~";
      rename(matrixpath.c_str(), backupmatrixpath.c_str());
    }
  std::string tempmatrix = rps_dumper_temporary_path(du, matrixpath);
  bool matrixok=
    mlpack::data::Save(tempmatrix, _machlearn_matrix, /*fatal:*/true,
                       /*transpose:*/false,
                       /*format:*/mlpack::data::FileType::CSVASCII);
  if (!matrixok)
    RPS_FATALOUT("failed to dump machine learning matrix "
                 << matrixpath << " for object " << this
                 << " in " << tempmatrix);
  std::string labelspath = labels_file_path();
  if (!access(labelspath.c_str(), F_OK))
    {
      std::string backuplabelspath = labelspath+"~";
      rename(labelspath.c_str(), backuplabelspath.c_str());
    }
  std::string templabels = rps_dumper_temporary_path(du, labelspath);
  bool labelsok=
    mlpack::data::Save(templabels, _machlearn_labels, /*fatal:*/true,
                       /*transpose:*/false,
                       /*format:*/mlpack::data::FileType::CSVASCII);
  if (!labelsok)
    RPS_FATALOUT("failed to dump machine learning labels "
                 << labelspath << " for object " << this
                 << " in " << templabels);
  jv["machlearn_matrix"] = matrixpath;
  jv["machlearn_labels"] = labelspath;
} // end Rps_PayloadMachLearn::dump_json_content

//// loading of Rps_PayloadMachLearn; see above Rps_PayloadMachLearn::dump_json_content
void
rpsldpy_machlearn(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  RPS_DEBUG_LOG(LOAD,"start rpsldpy_machlearn obz=" << obz
                << " jv=" << jv
                << " spacid=" << spacid
                << " lineno=" << lineno);
  auto paylmachlearn = obz->put_new_plain_payload<Rps_PayloadMachLearn>();
  RPS_ASSERT(paylmachlearn);
  std::string matrixpath = paylmachlearn->matrix_file_path();
  bool matrixok=
    mlpack::data::Load(matrixpath, paylmachlearn->_machlearn_matrix,
                       /*fatal:*/true,
                       /*transpose:*/false,
                       /*format:*/mlpack::data::FileType::CSVASCII);
  if (!matrixok)
    RPS_FATALOUT("failed to load machine learning matrix "
                 << matrixpath  << " for object " << obz);
  std::string labelspath = paylmachlearn->labels_file_path();
  bool labelsok=
    mlpack::data::Load(matrixpath, paylmachlearn->_machlearn_labels,
                       /*fatal:*/true,
                       /*transpose:*/false,
                       /*format:*/mlpack::data::FileType::CSVASCII);
  if (!labelsok)
    RPS_FATALOUT("failed to load machine learning labels "
                 << labelspath  << " for object " << obz);
} // end rpsldpy_machlearn

#warning very incomplete file machlearn_rps.cc

/// end of file machlearn_rps.cc

