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
    console.debug("ev=", ev, " inp=", inp, " text=", inp.text);
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
}                               // end object_input_keypress_event_wrps




/// the did_load_main_page_wrps is called by a script at end of main
/// page (index.html.rps)
function did_load_main_page_wrps()
{
    console.group("did_load_main_page_wrps");
    var showobj_inp = $('#showobj_inp_webrpsid');
    showobj_inp.keypress(object_input_keypress_event_wrps);
    console.debug("showobj_inp=", showobj_inp,
		  " with keypress handler object_input_keypress_event_wrps=",
		  object_input_keypress_event_wrps);
    console.groupEnd();
}                 // end did_load_main_page_wrps





/***
 * Basile don't understand a single line below, so comments it.
 * Several dozen words of written English (with URLs) are needed as
 * explanation.  The intuition remains: most keypresses should make an
 * AJAX request to RefPerSys.  These AJAX requests should be handled
 * by C++ code which would call (inside RefPerSys) the C++ functions
 * Rps_PayloadSymbol::autocomplete_name and
 * Rps_ObjectZone::autocomplete_oid...
 *
 * Of course that C++ code should be related to documented HTTP
 * requests. If possible, explain in written English the involved HTTP
 * request and the expected HTTP response (and their MIME content-type
 * and HTTP methods -for the request- and HTTP response code).
 ***/

//-// IMPORTANT NOTE: 
//-// The above two functions won't work because jQuery UI's $.autocomplete()
//-// function takes care of binding the required events $.autocomplete() function
//-// takes care of binding the required events. Instead, we need to define the
//-// required variables and functions required by $.autocomplete(). Also, jQuery
//-// UI seems to expect GET responses, and handling POST responses requires
//-// additional tweaking which I'm not aware of. Suggest for the demo that we use
//-// GET response for now, and then later see how we can use POST responses. In a
//-// demo, the client won't care about whether objects are being displayed as GET
//-// or POST requests.
//-// Also please note that jQuery UI enforces its own custom CSS, so our own CSS
//-// will probably conflict with it.
//-
$(document).ready(function () {
    console.group("document-ready");
    let $inp= $("#showobjinp_rpsid");
    /// https://www.tutsmake.com/how-to-get-the-current-page-url-path-host-using-jquery/
    let cururl = $(location).attr("href");
    console.debug(" $inp=", $inp, " arguments=", arguments,
		  " cururl=", cururl);
       $inp.autocomplete({
           source: function (request, response) {
               console.group("inp-autocomplete");
               console.debug(" autocompleting $inp=", $inp,
			     " cururl=", cururl,
                             " request=", request,
                             " response=", response,
                             " arguments=", arguments);
               $.ajax({
                   dataType: "json",

		   /// FIXME: probably autocompletion should use some
		   /// POST request, with the keyboard event?
                   type: "get",

                   //// FIXME: the localhost:9090 URL should not be hardcoded
                   //// below.  Perhaps that could be computed, maybe
                   //// from $inp....? And the getobject is *really*
                   //// confusing...., it probably should be complete_object
                   url: "http://localhost:9090/getobject", // replace URL

                   success: function (data) {
                       console.group("inp-autocomplete-success");
		       /// it is unclear why the removeClass below is
		       /// needed or useful...
                       ///? $inp.removeClass("ui-autocomplete-loading");
                       console.debug(" autocompleted success $inp=", $inp,
                                     " data=", data,
				     " cururl=", cururl,
                                     " arguments=", arguments,
                                     " request was:", request,
                                     " response was:", response);
                       response($.map(data, function (item) {
                           console.group("inp-autocomplete-response");
                           console.debug("data=", data, " response=", response,
                                         " arguments=", arguments,
					 " $inp=", inp,
					 " cururl=", cururl);
                           // TODO: Below, htm needs to contain the
                           // code for displaying the object details"
                           
                           let htm = "<h3>Showing object " 
                                   + item.oid 
                                   + "</h3>";
                           $lst.html(htm);
                           console.groupEnd();
                       }));
                       console.groupEnd();
                   },

                   error: function (data) {
                       console.group("inp-autocomplete-error");
                       /// again, localhost:9090 should not be hardcoded
                       console.log("http://localhost:9090/getobject needs to be implemented");
                       console.log("data=", data, " arguments=", arguments);
                       console.groupEnd();
                   }
               });
           },

           minLength: 3,

           open: function() {
               // TODO if required
               console.debug("inp-autocomplete-open arguments=", arguments,
                             " $inp=", $inp);
           },

           close: function() {
               // TODO if required
               console.debug("inp-autocomplete-close arguments=", arguments,
                             " $inp=", $inp);
           },

           focus: function(event, ui) {
               // TODO if required
               console.debug("inp-autocomplete-focus arguments=", arguments,
                             " $inp=", $inp);
           },

           select: function(event, ui) {
               // TODO if required
               console.debug("inp-autocomplete-select arguments=", arguments,
                             " $inp=", $inp);
           }
       });
    console.debug("after setting autocomplete $inp=", $inp,
                  " arguments=", arguments);
    console.groupEnd();
   });


console.log("file webroot/js/refpersys.js has been parsed");
// end file webroot/js/refpersys.js of refpersys.org
