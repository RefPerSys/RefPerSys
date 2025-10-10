// see http://refpersys.org/
// passed to commits after dd0c90db2992da (of Dec 28, 2022) of RefPerSys
// GPLv3+ licensed
// © Copyright 2023 - 2025 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin installs a commutative REPL operator
/*****
 * Once compiled, use it for example as:
 * ./refpersys --plugin-after-load=/tmp/rpsplug_createcommutativeoperator.so \
 *             --plugin-arg=rpsplug_createcommutativeoperator:++ \
 *             --extra=name=plusplus \
 *             --extra=precedence=8 \
 *             --batch --dump=.
 *
 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obnewoper;
                           Rps_ObjectRef obclasscommut;
                           Rps_ObjectRef obclassoper;
                           Rps_ObjectRef obclassrepldelim;
                           Rps_ObjectRef obreplprecedence;
                           Rps_Value strname;
                );
#define MYARGMAXLEN 64
  char argcopy[MYARGMAXLEN];
  bool argispunct = false;
  bool argisident = false;
  int precedence = -1;
  memset (argcopy, 0, MYARGMAXLEN);
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*xtraname = rps_get_extra_arg("name");
  const char*xtraprecedence = rps_get_extra_arg("precedence");
  const char*xtracomment = rps_get_extra_arg("comment");
  /// get repl_precedence attribute, conventionally values are small non-negative tagged integers
  _f.obreplprecedence = RPS_ROOT_OB(_7iVRsTR8u3D00Cy0hp); //repl_precedence∈symbol
  /// get the repl_operator superclass
  _f.obclassoper =  RPS_ROOT_OB(_9j12Nhm4itk00YYUW7); //repl_operator∈class
  _f.obclasscommut =  RPS_ROOT_OB(_2dvQOlSMjOu02zWx1n); //repl_commutative_operator∈class
  /// latter the class could be improved
  _f.obclassrepldelim = RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK); //repl_delimiter∈class
  RPS_ASSERT(_f.obclassoper);
  RPS_ASSERT(_f.obclasscommut);
  RPS_ASSERT(_f.obclassrepldelim);
  RPS_ASSERT(_f.obclassoper->is_class());
  RPS_ASSERT(_f.obclasscommut->is_class());
  RPS_ASSERT(_f.obclassrepldelim->is_class());
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some non-empty string");
  if (strlen(plugarg) >= MYARGMAXLEN-1)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " with too long argument " << Rps_QuotedC_String(plugarg));
  strncpy(argcopy, plugarg, MYARGMAXLEN);
  if (ispunct(plugarg[0]))
    {
      bool allpunct = true;
      for (const char*pc = plugarg; allpunct && *pc; pc++)
        allpunct = ispunct(*pc);
      argispunct = allpunct;
    };
  if (isalpha(plugarg[0]))
    {
      bool allident = true;
      for (const char*pc = plugarg; allident && *pc; pc++)
        allident = isalnum(*pc) || *pc=='_';
      argisident = allident;
    }
  if (xtraprecedence && isdigit(xtraprecedence[0]))
    precedence = atoi (xtraprecedence);
  if (!argispunct && !argisident)
    RPS_FATALOUT("rpsplug_createcommutativeoperator with bad argument "
                 <<  Rps_QuotedC_String(plugarg)
                 << " not identifier or all-delim");
  RPS_INFORMOUT("running plugin " << plugin->plugin_name << " with argument "
                << Rps_QuotedC_String(plugarg)
                << " and extra name " << Rps_QuotedC_String(xtraname)
                << " and extra precedence " << Rps_QuotedC_String(xtraprecedence));
  /** TODO:
   * We need to create the instance of _2dvQOlSMjOu02zWx1n //repl_commutative_operator∈class
   **/
  _f.obnewoper =
    Rps_ObjectRef::make_object(&_,
                               _f.obclasscommut,
                               Rps_ObjectRef::root_space());
  std::lock_guard<std::recursive_mutex> gunewoper(*_f.obnewoper->objmtxptr());
  /** TODO:
   * we need to fill obnewoper
   **/
  if (xtraname) {
    _f.namestr = Rps_Value{std::string(xtraname)};
    _f.obnewoper
      ->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
		 _f.namestr);
  }
  if (precedence >= 0) {
    _f.obnewoper->put_attr(RPS_ROOT_OB(_7iVRsTR8u3D00Cy0hp), //repl_precedence∈symbol
			   Rps_Value::make_tagged_int(precedence));
  }
  /** TODO:
   * we need to register obnewoper as a root or as a constant
   */
#warning unimplemented rpsplug_createcommutativeoperator
  RPS_FATALOUT("rpsplug_createcommutativeoperator not implemented for "
               <<  Rps_QuotedC_String(plugarg)
               << " but created " << RPS_OBJECT_DISPLAY(_f.obnewoper)
	       << " see rpsplug_thesetreploper.cc plugin");
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd $REFPERSYS_TOPDIR && ./do-build-refpersys-plugin -i plugins_dir/rpsplug_createcommutativeoperator.cc -o /tmp/rpsplug_createcommutativeoperator.so" ;;
 ** End: ;;
 ****************/
