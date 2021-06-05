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
function object_input_keypress_event_wrps(ev)
{
    console.group("object_input_keypress_event_wrps");
    var inp = ev.target;
    console.debug("ev=", ev, " inp=", inp);
    //// We probably need to make some AJAX call here doing
    //// autocompletion (in the RefPerSys server). And document it.
    ////
    //// We probably want some visual feedback. If a keypress is
    //// wrong, we might change the background color of the input.
    ////
    //// If it is complete and refers to only one object, we might
    //// also change the color.
    
    //// If it is not complete and refers to at most a dozen of
    //// objects, perhaps have some teardown menu....  If it is not
    //// complete and refers to many object, change the color
    //// differently.
    ////
    //// The related C++ functions in RefPerSys are
    //// Rps_ObjectZone::autocomplete_oid and
    //// Rps_PayloadSymbol::autocomplete_name
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


$(document).ready(function () {
    var $inp = $(); // TODO
});


// end file webroot/js/refpersys.js of refpersys.org
