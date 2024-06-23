// see http://refpersys.org/
// passed to commits after  9d1db4092 (of July 13, 2023)
// GPLv3+ licensed
// © Copyright 2023 - 2024 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin replace a root object by a constant rpskob_*
/*****

      Once compiled, use it for example as:

      ./refpersys --plugin-after-load=/tmp/rpsplug_root2const.so \
      --plugin-arg=rpsplug_root2const:oidorname \
      --extra=comment='some comment' \
      --batch --dump=.

****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obsymbol;
		 Rps_ObjectRef obsystem;
		 Rps_ObjectRef obnamedattr;
		 Rps_ObjectRef oboldroot;
		 Rps_ObjectRef obcomment;
		 Rps_Value namestr; // a string
		 Rps_Value commentstrv;
		 Rps_Value oldsetv;
		 Rps_Value newsetv;
		 );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*comment = rps_get_extra_arg("comment");
  const char*dumpdir = rps_get_extra_arg("dump");
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some name or root objectid");
  if (isalpha(plugarg[0])) {
    /// the argument is the name of some root object
    _f.oboldroot = Rps_PayloadSymbol::find_named_object(std::string(plugarg));
  }
  else if (plugarg[0] == '_' && isalnum(plugarg[1])) {
    _f.oboldroot = Rps_ObjectRef::find_object_or_null_by_oid(&_, Rps_Id(plugarg));
  }
  else
    RPS_FATALOUT("failure plugin " << plugin->plugin_name
		 << " with bad argument " << Rps_Cjson_String(plugarg)
		 << " (expect name or objid)");
  if (!_f.oboldroot) 
    RPS_FATALOUT("failure plugin " << plugin->plugin_name << " with argument " << plugarg
		 << " not refering to existing object");
  /// some "sacred" root objects cannot become constants
  else if (false
	   || _f.oboldroot == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
	   || _f.oboldroot == RPS_ROOT_OB(_10YXWeY7lYc01RpQTA) //the_system_class∈class
	   || _f.oboldroot == RPS_ROOT_OB(_1Io89yIORqn02SXx4p) //RefPerSys_system∈the_system_class
	   || _f.oboldroot == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
	   || _f.oboldroot == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) //int∈class
	   || _f.oboldroot == RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC) //"code_module"∈named_attribute
	   || _f.oboldroot == RPS_ROOT_OB(_3rXxMck40kz03RxRLM) //code_chunk∈class
	   || _f.oboldroot == RPS_ROOT_OB(_3s7ztCCoJsj04puTdQ) //agenda∈class
	   || _f.oboldroot == RPS_ROOT_OB(_3GHJQW0IIqS01QY8qD) //json∈class
	   || _f.oboldroot == RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5) //symbol∈symbol
	   || _f.oboldroot == RPS_ROOT_OB(_41OFI3r0S1t03qdB2E) //class∈class
	   || _f.oboldroot == RPS_ROOT_OB(_4jISxMJ4PYU0050nUl) //closure∈class
	   || _f.oboldroot == RPS_ROOT_OB(_4pSwobFHGf301Qgwzh) //named_attribute∈class
	   || _f.oboldroot == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ) //object∈class
	   || _f.oboldroot == RPS_ROOT_OB(_5CYWxcChKN002rw1fI) //contributor_to_RefPerSys∈class
	   || _f.oboldroot == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a) //environment∈class
	   || _f.oboldroot == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) //string∈class
	   || _f.oboldroot == RPS_ROOT_OB(_6fmq7pZkmNd03UyPuO) //class∈symbol
	   || _f.oboldroot == RPS_ROOT_OB(_6gxiw0snqrX01tZWW9) //"set_of_core_functions"∈mutable_set
	   || _f.oboldroot == RPS_ROOT_OB(_6ulDdOP2ZNr001cqVZ) //immutable_instance∈class
	   || _f.oboldroot == RPS_ROOT_OB(_6JYterg6iAu00cV9Ye) //set∈class
	   || _f.oboldroot == RPS_ROOT_OB(_6NVM7sMcITg01ug5TC) //tuple∈class
	   || _f.oboldroot == RPS_ROOT_OB(_6XLY6QfcDre02922jz) //value∈class
	   || _f.oboldroot == RPS_ROOT_OB(_7OrPRWQEg2o043XvK2) //rps_routine∈class
	   || _f.oboldroot == RPS_ROOT_OB(_7Y3AyF9gNx700bQJXc) //string_buffer∈class
	   || _f.oboldroot == RPS_ROOT_OB(_8fYqEw8vTED03wsznt) //tasklet∈class
	   || _f.oboldroot == RPS_ROOT_OB(_8J6vNYtP5E800eCr5q) //"initial_space"∈space
	   || _f.oboldroot == RPS_ROOT_OB(_98sc8kSOXV003i86w5) //double∈class
	   || _f.oboldroot == RPS_ROOT_OB(_9uwZtDshW4401x6MsY) //space∈symbol
	   || _f.oboldroot == RPS_ROOT_OB(_9BnrMLXUhfG00llx8X) //function∈class
	   || _f.oboldroot == RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS) //core_function∈class
	   /**** ease copy pasting in editor
	    **	 || _f.oboldroot == RPS_ROOT_OB(_)
	    **	 || _f.oboldroot == RPS_ROOT_OB(_)
	    **	 || _f.oboldroot == RPS_ROOT_OB(_)
	    **	 || _f.oboldroot == RPS_ROOT_OB(_)
	   *****/
	   ) {
    RPS_FATALOUT("failure plugin " << plugin->plugin_name << " with argument " << plugarg
		 << " refering to sacred root object " << _f.oboldroot
		 << std::endl
		 << "Please edit " << __FILE__);
  };
  _f.obsystem = RPS_ROOT_OB(_1Io89yIORqn02SXx4p); //RefPerSys_system∈the_system_class
  std::lock_guard<std::recursive_mutex> gu(*_f.obsystem->objmtxptr());
  _f.oldsetv = _f.obsystem->get_physical_attr(RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp)); // //"constant"∈named_attribute
  RPS_ASSERT(_f.oldsetv.is_set());
  _f.newsetv = Rps_SetValue{_f.oldsetv, Rps_Value(_f.oboldroot)};
  RPS_ASSERT(_f.newsetv.as_set()->cardinal() >= _f.oldsetv.as_set()->cardinal());
  if (comment && comment[0]) { ///if some comment is given put it
    _f.commentstrv = Rps_StringValue(comment);
    std::lock_guard<std::recursive_mutex> gu(*_f.oboldroot->objmtxptr());
    _f.oboldroot->put_attr(RPS_ROOT_OB(_0jdbikGJFq100dgX1n), //comment∈symbol
			   _f.commentstrv);
    _f.oboldroot->touch_now();
  };
  /// update the set of contants
  _f.obsystem->put_attr(RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp), // //"constant"∈named_attribute
			_f.newsetv);
  /// remove the root object
  if (!rps_remove_root_object(_f.oboldroot))
    RPS_WARNOUT("plugin " << plugin->plugin_name
		<< " failed to remove non-root object " << _f.oboldroot
		<< std::endl << " but did register it as a constant");
  RPS_INFORMOUT("plugin " << plugin->plugin_name
		<< " had old constant set " << _f.oldsetv
		<< " did move old root " << _f.oboldroot
		<< " in attribute " << RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp)
		<< " of " << RPS_ROOT_OB(_1Io89yIORqn02SXx4p) << std::endl
	      << " new constant set newsetv=" << _f.newsetv);
  if (dumpdir) {
    RPS_INFORMOUT("plugin " << plugin->plugin_name << " dumping to " << dumpdir);
    rps_dump_into (std::string(dumpdir), &_);
    RPS_INFORMOUT("plugin " << plugin->plugin_name << " dumped to " << dumpdir);
  }
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./build-plugin.sh plugins_dir/rpsplug_root2const.cc /tmp/rpsplug_root2const.so" ;;
 ** End: ;;
 ****************/
