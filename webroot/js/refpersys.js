// file webroot/js/refpersys.js
// SPDX-License-Identifier: GPL-3.0-or-later
//
//      This file is part of the Reflective Persistent System. See
//      website refpersys.org for more.
//
//      It holds the hand written JavaScript code when
//      e.g. http://localhost:9090/ is accessed, and when
//      ./refpersys -W. is running.
//
// Author(s):
//    Basile Starynkevitch <basile@starynkevitch.net>
//    Abhishek Chakravarti <abhishek@taranjali.org>
//    Nimesh Neema <nimeshneema@gmail.com>
//
//    © Copyright 2020 - 2021 The Reflective Persistent System Team
//    team@refpersys.org & http://refpersys.org/

// the Jquery framework has been loaded and can be used.

/// By convention Javascript functions here have a name ending withj wrps

/// Jquery function to be called on every keypress event for all input
/// elements for a RefPerSys object.
function object_input_keypress_event_wrps(ev,w)
{
    console.group("object_input_keypress_event_wrps");
    console.debug("ev=", ev, " w=", w);
    console.groupEnd();
}				// end object_input_keypress_event_wrps




/// the did_load_main_page_wrps is called by a script at end of main
/// page (index.html.rps)
function did_load_main_page_wrps()
{
    console.group("did_load_main_page_wrps");
    var showobj_inp = $('#showobj_inp_webrpsid');
    showobj_inp.keypress(object_input_keypress_event_wrps);
    console.debug("showobj_inp=", showobj_inp);
    console.groupEnd();
}		  // end did_load_main_page_wrps

// end file webroot/js/refpersys.js of refpersys.org
