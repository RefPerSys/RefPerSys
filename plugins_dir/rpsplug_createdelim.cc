// see http://refpersys.org/
// passed to commits after 7d645156f6bb3d  (of April, 26, 2025) of RefPerSys
// GPLv3+ licensed
// © Copyright 2025 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin installs a REPL delimiter
/*****
 * Once compiled, use it for example as:
 * ./refpersys --plugin-after-load=/tmp/rpsplug_createdelim.so \
 *             --plugin-arg=rpsplug_createdelim:++ \
 *             --extra=name=plusplus \
 *             --extra=precedence=8 \
 *             --batch --dump=.
 *
 ****/

#include "refpersys.hh"

extern "C" const char rpsplugdelim_gitid[];
extern "C" const char rpsplugdelim_shortgit[];
extern "C" const char rpsplugdelim_date[];

const char rpsplugdelim_gitid[]=RPS_GITID;
const char rpsplugdelim_shortgit[]= RPS_SHORTGIT;
const char rpsplugdelim_date[]=__DATE__;

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obnewroot;
                           Rps_ObjectRef obclasscommut;
                           Rps_ObjectRef obclassoper;
                           Rps_ObjectRef obclassrepldelim;
                           Rps_ObjectRef obdelim;
                           Rps_ObjectRef obsymbol;
                           Rps_ObjectRef obold;
                           Rps_ObjectRef obdictdelim;
                           Rps_ObjectRef obreplprecedence;
                           Rps_Value strname;
                           Rps_Value strdelim;
                           Rps_Value strcomment;
                );
#define MYARGMAXLEN 64
  char argcopy[MYARGMAXLEN];
  bool argispunct = false;
  bool argisident = false;
  memset (argcopy, 0, MYARGMAXLEN);
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*xtraname = rps_get_extra_arg("name");
  const char*comment = rps_get_extra_arg("comment");
  RPS_INFORMOUT("running plugin " << plugin->plugin_name << " with argument "
                << Rps_QuotedC_String(plugarg)
                << " and extra name " << Rps_QuotedC_String(xtraname));
  _f.obclassrepldelim = RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK); //repl_delimiter∈class
  RPS_ASSERT(_f.obclassrepldelim);
  _f.obdictdelim = RPS_ROOT_OB(_627ngdqrVfF020ugC5); //"repl_delim"∈string_dictionary
  RPS_ASSERT(_f.obdictdelim);
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
    }
  else if (isalpha(plugarg[0]))
    {
      bool allident = true;
      for (const char*pc = plugarg; allident && *pc; pc++)
        allident = isalnum(*pc) || *pc=='_';
      argisident = allident;
    }
  if (!plugarg[0] && !argispunct && !argisident)
    RPS_FATALOUT("rpsplug_createdelim with bad argument "
                 <<  Rps_QuotedC_String(plugarg)
                 << " not identifier or all-delim");
  _f.strdelim = Rps_StringValue(plugarg);
  std::lock_guard<std::recursive_mutex> gudictdelim(*_f.obdictdelim->objmtxptr());
  _f.obdelim =
    Rps_ObjectRef::make_object(&_,
                               _f.obclassrepldelim,
                               Rps_ObjectRef::root_space());
  std::lock_guard<std::recursive_mutex> gunewdelim(*_f.obdelim->objmtxptr());
  auto paylstrdict = _f.obdictdelim->get_dynamic_payload<Rps_PayloadStringDict>();
  if (!paylstrdict)
    RPS_FATALOUT("the delimiter dictionary " << _f.obdictdelim << " has a wrong payload");
  paylstrdict->add(plugarg, _f.obdelim);
  _f.obdelim->put_attr(RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK), //repl_delimiter∈class
                       _f.strdelim);
  if (xtraname && isalpha(xtraname[0]))
    {
      if (!Rps_PayloadSymbol::valid_name(xtraname))
        RPS_FATALOUT("The name '" << Rps_QuotedC_String(xtraname)
                     << "' is invalid");
      _f.obold = Rps_PayloadSymbol::find_named_object(xtraname);
      if (_f.obold)
        RPS_FATALOUT("The name '" << Rps_QuotedC_String(xtraname)
                     << "' is already used by " << _f.obold);
      _f.strname = Rps_StringValue(xtraname);
      _f.obsymbol = Rps_ObjectRef::make_new_strong_symbol(&_, xtraname);
      _f.obsymbol->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
                            _f.strname);
      _f.obsymbol->put_attr(RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK), //repl_delimiter∈class
                            _f.obdelim);
      _f.obdelim->put_attr2(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
                            _f.strname,
                            RPS_ROOT_OB(_36I1BY2NetN03WjrOv), //symbol∈class
                            _f.obsymbol);
    }
  if (comment && comment[0])
    {
      _f.strcomment = Rps_StringValue(comment);
      _f.obsymbol->put_attr(RPS_ROOT_OB(_0jdbikGJFq100dgX1n), //comment∈symbol)
                            _f.strcomment);
    }
  rps_add_constant_object(&_, _f.obdelim);
  RPS_INFORMOUT("plugin " << plugin->plugin_name
                << " created constant delimiter:" << std::endl
                << RPS_OBJECT_DISPLAY(_f.obdelim)
                << std::endl
                << "... with symbol:" << std::endl
                << RPS_OBJECT_DISPLAY(_f.obsymbol)
                << std::endl);
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./do-build-refpersys-plugin -v plugins_dir/rpsplug_createdelim.cc -o /tmp/rpsplug_createdelim.so" ;;
 ** End: ;;
 ****************/
