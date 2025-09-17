// see http://refpersys.org/
// passed to commits after 306c6f5f688a (in august 2025) of RefPerSys
// GPLv3+ licensed
// © Copyright 2025 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin rpsplug_createmonoper.cc installs a monoadic unary REPL operator
/*****
 * Once compiled, use it for example as:
 * ./refpersys --plugin-after-load=/tmp/rpsplug_createmonoper.so \
 *             --plugin-arg=rpsplug_createmonoper:- \
 *             --extra=name=negate \
 *             --extra=precedence=8 \
 *             --extra=comment='unary negation operator'
 *             --batch --dump=.
 *
 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obnewoper;
                           Rps_ObjectRef obsymbol;
                           Rps_ObjectRef obclassbinary;
                           Rps_ObjectRef obclassoper;
                           Rps_ObjectRef obclassrepldelim;
                           Rps_ObjectRef obreplprecedence;
                           Rps_Value strname;
                           Rps_Value strcomment;
                );
#define MYARGMAXLEN 64
  char argcopy[MYARGMAXLEN];
  bool argispunct = false;
  bool argisident = false;
  int precedence = -1;
  const char*rooted = rps_get_extra_arg("rooted");
  const char*constant = rps_get_extra_arg("constant");
  const char*comment = rps_get_extra_arg("comment");
  const char*xtraname = rps_get_extra_arg("name");
  const char*xtraprecedence = rps_get_extra_arg("precedence");
  bool isrooted = false;
  bool isconstant = false;
  memset (argcopy, 0, MYARGMAXLEN);
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some non-empty string");
  if (strlen(plugarg) >= MYARGMAXLEN-1)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " with too long argument " << Rps_QuotedC_String(plugarg));
  if (rooted)
    {
      if (!strcmp(rooted, "true"))
        isrooted = true;
      if (atoi(rooted) > 0)
        isrooted = true;
    }
  else if (constant)
    {
      if (!strcmp(constant, "true"))
        isconstant = true;
      if (atoi(constant) > 0)
        isconstant = true;
    };
  /// get repl_precedence attribute, conventionally values are small
  /// non-negative tagged integers
  _f.obreplprecedence = RPS_ROOT_OB(_7iVRsTR8u3D00Cy0hp); //repl_precedence∈symbol
  /// get the repl_operator superclass
  _f.obclassoper =  RPS_ROOT_OB(_6vcJz35mTam01zYLjL); //repl_unary_operator∈class
  RPS_ASSERT(_f.obclassoper->is_class());
  _f.obclassrepldelim = RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK); //repl_delimiter∈class
  RPS_ASSERT(_f.obclassoper);
  /** we might improve and later accept a subclass for the operator **/
  RPS_ASSERT(_f.obclassoper->is_class());
  RPS_ASSERT(_f.obclassrepldelim);
  RPS_ASSERT(_f.obclassrepldelim->is_class());
  strncpy(argcopy, plugarg, MYARGMAXLEN);
  if (xtraprecedence && isdigit(xtraprecedence[0]))
    precedence = atoi (xtraprecedence);
  else
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " with bad precedence " << precedence << " for argument " << Rps_QuotedC_String(plugarg));

  RPS_ASSERT(_f.obclassoper->is_class());
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
    };
  if (!argispunct && !argisident)
    RPS_FATALOUT("rpsplug_createmonoper with bad argument "
                 <<  Rps_QuotedC_String(plugarg)
                 << " not identifier or all-delim");
  RPS_INFORMOUT("running plugin " << plugin->plugin_name << " with argument "
                << Rps_QuotedC_String(plugarg)
                << " and extra name " << Rps_QuotedC_String(xtraname)
                << " and extra precedence " << Rps_QuotedC_String(xtraprecedence));
  /*** TODO:
   * We need to create the instance of _55Z5Wgzuprq01MU6Br //repl_binary_operator∈class
   ****/
  _f.obnewoper =
    Rps_ObjectRef::make_object(&_,
                               _f.obclassoper,
                               Rps_ObjectRef::root_space());
  std::lock_guard<std::recursive_mutex> gunewoper(*_f.obnewoper->objmtxptr());
  /** TODO:
   * we need to fill obnewoper and register it as a root or as a constant
   */
  if (xtraname)
    {
      _f.strname = Rps_Value{std::string(xtraname)};
      _f.obnewoper
      ->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
                 _f.strname);
    }
  if (precedence >= 0)
    {
      _f.obnewoper->put_attr(RPS_ROOT_OB(_7iVRsTR8u3D00Cy0hp), //repl_precedence∈symbol
                             Rps_Value::make_tagged_int(precedence));
    }
  if (comment != nullptr)
    {
      _f.strcomment = Rps_StringValue(comment);
      _f.obnewoper->put_attr(PS_ROOT_OB(_0jdbikGJFq100dgX1n), //comment∈symbol
                             _f.strcomment);
    }
  if (isrooted)
    {
      rps_add_root_object (_f.obnewoper);
      RPS_INFORMOUT("plugin " << plugin->plugin_name
                    << " created root monadic REPL operator"
                    << std::endl
                    << RPS_OBJECT_DISPLAY(_f.obnewoper));
      return;
    }
  else if (isconstant)
    {
      rps_add_constant_object(&_, _f.obnewoper);
      RPS_INFORMOUT("plugin " << plugin->plugin_name
                    << " created constant monadic REPL operator"
                    << std::endl
                    << RPS_OBJECT_DISPLAY(_f.obnewoper));
      return;
    }
  else
    /***
     *
     * A possible way of compiling this plugin might be to run:
     *
      make one-plugin REFPERSYS_PLUGIN_SOURCE=$REFPERSYS_TOPDIR/plugins_dir/rpsplug_createmonoper.cc \
                      REFPERSYS_PLUGIN_SHARED_OBJECT=/tmp/rpsplug_createmonoper.so
     *
     **/
    RPS_FATALOUT("rpsplug_createmonoper not implemented for "
                 <<  Rps_QuotedC_String(plugarg)
                 << " but created " << RPS_OBJECT_DISPLAY(_f.obnewoper)
                 << " see rpsplug_thesetreploper.cc plugin");
#warning incomplete rpsplug_createmonoper
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd $REFPERSYS_TOPDIR && ./do-build-refpersys-plugin -v plugins_dir/rpsplug_createmonoper.cc -o /tmp/rpsplug_createmonoper.so" ;;
 ** End: ;;
 ****************/
