// see http://refpersys.org/
// passed to commits after ae10f76190d5 (of Dec 30, 2022) of RefPerSys
// GPLv3+ licensed
// © Copyright 2022 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin creates the repl_commutative_operator_class
/*****
 * Once compiled, use it for example as:
 * ./refpersys --plugin-after-load=/tmp/rpsplug_createcommutativeclass.so \
 *             --batch --dump=.                                                
 *
 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obnewroot;
		 Rps_ObjectRef obcommutclass;
		 Rps_ObjectRef obclassoper;
                 Rps_Value strname;
                );
  /// get the repl_operator superclass
  _f.obclassoper =  RPS_ROOT_OB(_9j12Nhm4itk00YYUW7); //repl_operator∈class
  RPS_ASSERT(_f.obclassoper);
  RPS_ASSERT(_f.obclassoper->is_class());
  _f.obcommutclass = Rps_ObjectRef::make_named_class(&_,
						     _f.obclassoper,
						     std::string{"repl_commutative_operator_class"});
  _f.strname = Rps_String::make("repl_commutative_operator_class");
  _f.obcommutclass->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
				     _f.strname);
  rps_add_root_object(_f.obcommutclass);
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/local/include  -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -Og -g -shared -fPIC $HOME/tmp/rpsplug_createcommutativeclass.cc -o /tmp/rpsplug_createcommutativeclass.so" ;;
 ** End: ;;
 ****************/

