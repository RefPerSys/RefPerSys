/****************************************************************
 * file output_qrps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It holds the Qt5 code related to the Qt5 output widget
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2020 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
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
#include "qthead_qrps.hh"


extern "C" const char rps_output_gitid[];
const char rps_output_gitid[]= RPS_GITID;

extern "C" const char rps_output_date[];
const char rps_output_date[]= __DATE__;


//////////////////////////////////////////////////////////////// RpsQOutputTextEdit

QTextCharFormat RpsQOutputTextEdit::outptxt_int_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_double_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_string_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_tuple_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_set_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_anonymousobject_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_empty_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_etc_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_qtptr_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_json_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_symbol_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_oid_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_class_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_closure_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_instance_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_metadata_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_objecttitle_qcfmt_;
QTextCharFormat RpsQOutputTextEdit::outptxt_objectdecor_qcfmt_;
QTextFrameFormat RpsQOutputTextEdit::outptxt_objectcontent_qfrfmt_;


void
RpsQOutputTextEdit::initialize()
{
  QSettings* qst = RpsQApplication::qt_settings();
  RPS_ASSERT(qst);

  /// how to display integer values
  {
    QColor int_bgcol = qst->value("out/int/bgcolor").value<QColor>();
    outptxt_int_qcfmt_.setBackground(QBrush(int_bgcol));
    QColor int_fgcol = qst->value("out/int/fgcolor").value<QColor>();
    outptxt_int_qcfmt_.setForeground(QBrush(int_fgcol));
    QFont int_font = qst->value("out/int/font").value<QFont>();
    outptxt_int_qcfmt_.setFont(int_font);
  }

  /// how to display double values
  {
    QColor double_bgcol = qst->value("out/double/bgcolor").value<QColor>();
    outptxt_double_qcfmt_.setBackground(QBrush(double_bgcol));
    QColor double_fgcol = qst->value("out/double/fgcolor").value<QColor>();
    outptxt_double_qcfmt_.setForeground(QBrush(double_fgcol));
    QFont double_font = qst->value("out/double/font").value<QFont>();
    outptxt_double_qcfmt_.setFont(double_font);
  }

  /// how to display string values
  {
    QColor string_bgcol = qst->value("out/string/bgcolor").value<QColor>();
    outptxt_string_qcfmt_.setBackground(QBrush(string_bgcol));
    QColor string_fgcol = qst->value("out/string/fgcolor").value<QColor>();
    outptxt_string_qcfmt_.setForeground(QBrush(string_fgcol));
    QFont string_font = qst->value("out/string/font").value<QFont>();
    outptxt_string_qcfmt_.setFont(string_font);
  }

  /// how to display tuple values
  {
    QColor tuple_bgcol = qst->value("out/tuple/bgcolor").value<QColor>();
    outptxt_tuple_qcfmt_.setBackground(QBrush(tuple_bgcol));
    QColor tuple_fgcol = qst->value("out/tuple/fgcolor").value<QColor>();
    outptxt_tuple_qcfmt_.setForeground(QBrush(tuple_fgcol));
    QFont tuple_font = qst->value("out/tuple/font").value<QFont>();
    outptxt_tuple_qcfmt_.setFont(tuple_font);
  }
  /// how to display set values
  {
    QColor set_bgcol = qst->value("out/set/bgcolor").value<QColor>();
    outptxt_set_qcfmt_.setBackground(QBrush(set_bgcol));
    QColor set_fgcol = qst->value("out/set/fgcolor").value<QColor>();
    outptxt_set_qcfmt_.setForeground(QBrush(set_fgcol));
    QFont set_font = qst->value("out/set/font").value<QFont>();
    outptxt_set_qcfmt_.setFont(set_font);
  }
  /// how to display anonymous object references
  {
    QColor anonymousobject_bgcol = qst->value("out/anonymousobject/bgcolor").value<QColor>();
    outptxt_anonymousobject_qcfmt_.setBackground(QBrush(anonymousobject_bgcol));
    QColor anonymousobject_fgcol = qst->value("out/anonymousobject/fgcolor").value<QColor>();
    outptxt_anonymousobject_qcfmt_.setForeground(QBrush(anonymousobject_fgcol));
    QFont anonymousobject_font = qst->value("out/anonymousobject/font").value<QFont>();
    outptxt_anonymousobject_qcfmt_.setFont(anonymousobject_font);
  }
  /// how to display empty stuff
  {
    QColor empty_bgcol = qst->value("out/empty/bgcolor").value<QColor>();
    outptxt_empty_qcfmt_.setBackground(QBrush(empty_bgcol));
    QColor empty_fgcol = qst->value("out/empty/fgcolor").value<QColor>();
    outptxt_empty_qcfmt_.setForeground(QBrush(empty_fgcol));
    QFont empty_font = qst->value("out/empty/font").value<QFont>();
    outptxt_empty_qcfmt_.setFont(empty_font);
  }
  /// how to display ellipsis for too deep things
  {
    QColor etc_bgcol = qst->value("out/etc/bgcolor").value<QColor>();
    outptxt_etc_qcfmt_.setBackground(QBrush(etc_bgcol));
    QColor etc_fgcol = qst->value("out/etc/fgcolor").value<QColor>();
    outptxt_etc_qcfmt_.setForeground(QBrush(etc_fgcol));
    QFont etc_font = qst->value("out/etc/font").value<QFont>();
    outptxt_etc_qcfmt_.setFont(etc_font);
  }
  /// how to display qtptr values
  {
    QColor qtptr_bgcol = qst->value("out/qtptr/bgcolor").value<QColor>();
    outptxt_qtptr_qcfmt_.setBackground(QBrush(qtptr_bgcol));
    QColor qtptr_fgcol = qst->value("out/qtptr/fgcolor").value<QColor>();
    outptxt_qtptr_qcfmt_.setForeground(QBrush(qtptr_fgcol));
    QFont qtptr_font = qst->value("out/qtptr/font").value<QFont>();
    outptxt_qtptr_qcfmt_.setFont(qtptr_font);
  }
  /// how to display json values
  {
    QColor json_bgcol = qst->value("out/json/bgcolor").value<QColor>();
    outptxt_json_qcfmt_.setBackground(QBrush(json_bgcol));
    QColor json_fgcol = qst->value("out/json/fgcolor").value<QColor>();
    outptxt_json_qcfmt_.setForeground(QBrush(json_fgcol));
    QFont json_font = qst->value("out/json/font").value<QFont>();
    outptxt_json_qcfmt_.setFont(json_font);
  }
  /// how to display symbol values
  {
    QColor symbol_bgcol = qst->value("out/symbol/bgcolor").value<QColor>();
    outptxt_symbol_qcfmt_.setBackground(QBrush(symbol_bgcol));
    QColor symbol_fgcol = qst->value("out/symbol/fgcolor").value<QColor>();
    outptxt_symbol_qcfmt_.setForeground(QBrush(symbol_fgcol));
    QFont symbol_font = qst->value("out/symbol/font").value<QFont>();
    outptxt_symbol_qcfmt_.setFont(symbol_font);
  }
  /// how to display oid values
  {
    QColor oid_bgcol = qst->value("out/oid/bgcolor").value<QColor>();
    outptxt_oid_qcfmt_.setBackground(QBrush(oid_bgcol));
    QColor oid_fgcol = qst->value("out/oid/fgcolor").value<QColor>();
    outptxt_oid_qcfmt_.setForeground(QBrush(oid_fgcol));
    QFont oid_font = qst->value("out/oid/font").value<QFont>();
    outptxt_oid_qcfmt_.setFont(oid_font);
  }
  /// how to display classes
  {
    QColor class_bgcol = qst->value("out/class/bgcolor").value<QColor>();
    outptxt_class_qcfmt_.setBackground(QBrush(class_bgcol));
    QColor class_fgcol = qst->value("out/class/fgcolor").value<QColor>();
    outptxt_class_qcfmt_.setForeground(QBrush(class_fgcol));
    QFont class_font = qst->value("out/class/font").value<QFont>();
    outptxt_class_qcfmt_.setFont(class_font);
  }
  /// how to display closures
  {
    QColor closure_bgcol = qst->value("out/closure/bgcolor").value<QColor>();
    outptxt_closure_qcfmt_.setBackground(QBrush(closure_bgcol));
    QColor closure_fgcol = qst->value("out/closure/fgcolor").value<QColor>();
    outptxt_closure_qcfmt_.setForeground(QBrush(closure_fgcol));
    QFont closure_font = qst->value("out/closure/font").value<QFont>();
    outptxt_closure_qcfmt_.setFont(closure_font);
  }
  /// how to display instances
  {
    QColor instance_bgcol = qst->value("out/instance/bgcolor").value<QColor>();
    outptxt_instance_qcfmt_.setBackground(QBrush(instance_bgcol));
    QColor instance_fgcol = qst->value("out/instance/fgcolor").value<QColor>();
    outptxt_instance_qcfmt_.setForeground(QBrush(instance_fgcol));
    QFont instance_font = qst->value("out/instance/font").value<QFont>();
    outptxt_instance_qcfmt_.setFont(instance_font);
  }
  /// how to display metadatas
  {
    QColor metadata_bgcol = qst->value("out/metadata/bgcolor").value<QColor>();
    outptxt_metadata_qcfmt_.setBackground(QBrush(metadata_bgcol));
    QColor metadata_fgcol = qst->value("out/metadata/fgcolor").value<QColor>();
    outptxt_metadata_qcfmt_.setForeground(QBrush(metadata_fgcol));
    QFont metadata_font = qst->value("out/metadata/font").value<QFont>();
    outptxt_metadata_qcfmt_.setFont(metadata_font);
  }
  /// how to display objecttitle
  {
    QColor objecttitle_bgcol = qst->value("out/objecttitle/bgcolor").value<QColor>();
    outptxt_objecttitle_qcfmt_.setBackground(QBrush(objecttitle_bgcol));
    QColor objecttitle_fgcol = qst->value("out/objecttitle/fgcolor").value<QColor>();
    outptxt_objecttitle_qcfmt_.setForeground(QBrush(objecttitle_fgcol));
    QFont objecttitle_font = qst->value("out/objecttitle/font").value<QFont>();
    outptxt_objecttitle_qcfmt_.setFont(objecttitle_font);
  }
  /// how to display object decor
  {
    QColor objectdecor_bgcol = qst->value("out/objectdecor/bgcolor").value<QColor>();
    outptxt_objectdecor_qcfmt_.setBackground(QBrush(objectdecor_bgcol));
    QColor objectdecor_fgcol = qst->value("out/objectdecor/fgcolor").value<QColor>();
    outptxt_objectdecor_qcfmt_.setForeground(QBrush(objectdecor_fgcol));
    QFont objectdecor_font = qst->value("out/objectdecor/font").value<QFont>();
    outptxt_objectdecor_qcfmt_.setFont(objectdecor_font);
  }
  //// frame format for object contents
  {
    QColor objectcontent_bgcol = qst->value("out/objectcontent/bgcolor").value<QColor>();
    outptxt_objectcontent_qfrfmt_.setBackground(QBrush(objectcontent_bgcol));
    QColor objectcontent_fgcol = qst->value("out/objectcontent/fgcolor").value<QColor>();
    outptxt_objectcontent_qfrfmt_.setBackground(QBrush(objectcontent_fgcol));
    qreal objectcontent_margin = qst->value("out/objectcontent/margin").toDouble();
    outptxt_objectcontent_qfrfmt_.setMargin(objectcontent_margin);
    qreal objectcontent_padding = qst->value("out/objectcontent/padding").toDouble();
    outptxt_objectcontent_qfrfmt_.setPadding(objectcontent_padding);
    qreal objectcontent_border = qst->value("out/objectcontent/border").toDouble();
    outptxt_objectcontent_qfrfmt_.setBorder(objectcontent_border);
    outptxt_objectcontent_qfrfmt_.setBorderStyle(QTextFrameFormat::BorderStyle_Dotted);
  }
#warning more is needed in RpsQOutputTextEdit::initialize
} // end RpsQOutputTextEdit::initialize




RpsQOutputTextEdit::RpsQOutputTextEdit(QWidget*parent)
  : QTextEdit(parent),
    outptxt_objref(),
    outptxt_maxdepth(default_maximal_depth),
    outptxt_columnthresh(default_column_threshold)
{
  setDocumentTitle("output");
} // end RpsQOutputTextEdit::RpsQOutputTextEdit

RpsQOutputTextEdit::~RpsQOutputTextEdit()
{
} // end RpsQOutputTextEdit::~RpsQOutputTextEdit


void
RpsQOutputTextEdit::create_outpedit_object(Rps_CallFrame*callerframe)
{
  RPS_LOCALFRAME(nullptr /*no descr*/,
                 callerframe,
                 Rps_ObjectRef obtxed;
                );
  RPS_ASSERT(!outptxt_objref);
  _.obtxed =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_1NWEOIzo3WU03mE42Q) /*rps_output_textedit class*/);
  auto paylt = _.obtxed->put_new_plain_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  paylt->set_qtptr(this);
  RPS_DEBUG_LOG(GUI, "RpsQOutputTextEdit::create_outpedit_object obtxed=" << _.obtxed);
  outptxt_objref = _.obtxed;
} // end RpsQOutputTextEdit::create_outpedit_object

void
RpsQOutputTextEdit::output_space_or_indented_newline(QTextCharFormat qc, int depth)
{
  if (depth<0)
    depth=0;
  else if (depth>outptxt_maxdepth)
    depth=outptxt_maxdepth;
  auto qcursor = textCursor();
  if (qcursor.columnNumber() > outptxt_columnthresh)
    {
      qcursor.insertText("\n", qc);
      static const char spaces16[] = "                ";
      RPS_ASSERT(strlen(spaces16)==16);
      while (depth>16)
        {
          qcursor.insertText(spaces16, qc);
          depth -= 16;
        }
      qcursor.insertText(spaces16+16-depth, qc);
    }
  else
    qcursor.insertText(" ", qc);
} // end RpsQOutputTextEdit::output_space_or_indented_newline

////////////////
//// display a value, maybe even nil, into this output text edit, at given depth. May throw some exception
void
RpsQOutputTextEdit::display_output_value(Rps_CallFrame*callerframe, const Rps_Value valuearg, int depth)
{
  RPS_LOCALFRAME(rpskob_1Win5yzaf1L02cBUlV, //display_value_qt
                 //selector
                 callerframe, //
                 Rps_ObjectRef winob;
                 Rps_Value dispval;
                );
  _.winob = outptxt_objref.as_object();
  _.dispval = valuearg;
  RPS_ASSERT(_.winob);
  std::lock_guard<std::recursive_mutex> guobwin (*(_.winob->objmtxptr()));
  auto qoutwpayl =
    _.winob->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT(qoutwpayl);
  RpsQOutputTextEdit* qoutxed = qoutwpayl->qtptr();
  RPS_ASSERT(qoutxed == this);
  // we should special-case when _.dispval is empty and have some
  // outptxt_empty_qcfmt_ for that case.
  if (_.dispval.is_empty())
    {
      auto qcfmt = RpsQOutputTextEdit::empty_text_format();
      // we display a white rectangle
      auto qcursout = qoutxed->textCursor();
      qcursout.insertText(QString("▭"), //U+25AD WHITE RECTANGLE
                          qcfmt);
    }
  else if (depth >  max_output_depth())
    {
      auto qcfmt = RpsQOutputTextEdit::etc_text_format();
      // we display an ellipsis
      auto qcursout = qoutxed->textCursor();
      qcursout.insertText(QString("…"), //U+2026 HORIZONTAL ELLIPSIS
                          qcfmt);
    }
  else // non-empty _.dispval
    {
      // Otherwise send RefPerSys selector display_value_qt of oid
      // _1Win5yzaf1L02cBUlV to winob, dispval, depth...
      Rps_ObjectRef selob_display_value_qt =
        RPS_ROOT_OB(_1Win5yzaf1L02cBUlV);
      Rps_TwoValues respair =
        Rps_ObjectValue(_.winob).send2(&_, selob_display_value_qt,
                                       _.dispval, Rps_Value((intptr_t)depth));
      if (!respair)
        throw RPS_RUNTIME_ERROR_OUT("display_output_value failed winob="
                                    << _.winob
                                    << " dispval=" << _.dispval
                                    << " depth#" << depth);
    };				// end if _.dispval non-empty
} // end RpsQOutputTextEdit::display_output_value



void
rps_display_output_value(Rps_CallFrame*callerframe,
                         Rps_ObjectRef argobwin, const Rps_Value argvalue, int depth)
{
  RPS_LOCALFRAME(rpskob_1Win5yzaf1L02cBUlV, //display_value_qt
                 //selector
                 callerframe, //
                 Rps_ObjectRef winob;
                 Rps_Value dispval;
                );
  _.winob = argobwin;
  _.dispval = argvalue;
  RPS_DEBUG_LOG(GUI, "rps_display_output_value start winob=" << _.winob
                << ", dispval=" << _.dispval
                << ", depth=" << depth);
  RPS_ASSERT(_.winob);
  std::lock_guard<std::recursive_mutex> guobwin (*(_.winob->objmtxptr()));
  auto qoutwpayl =
    _.winob->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT(qoutwpayl);
  RpsQOutputTextEdit* qoutxed = qoutwpayl->qtptr();
  RPS_ASSERT(qoutxed);
  qoutxed->display_output_value(&_, _.dispval, depth);
  RPS_DEBUG_LOG(GUI, "rps_display_output_value end winob=" << _.winob
                << ", dispval=" << _.dispval
                << ", depth=" << depth);
} // end rps_display_output_value




////////////////
//// display an object occurrence or nil, into this output text edit, at given depth. May throw some exception
void
RpsQOutputTextEdit::display_output_object_occurrence(Rps_CallFrame*callerframe, const Rps_ObjectRef argdispobr, int depth)
{
  RPS_LOCALFRAME(rpskob_4ojpzRzyRWz02DNWMe, //display_object_occurrence_qt
                 //selector
                 callerframe, //
                 Rps_ObjectRef winob;
                 Rps_ObjectRef dispob;
                );
  _.winob = outptxt_objref.as_object();
  _.dispob = argdispobr;
  RPS_ASSERT(_.winob);
  RPS_DEBUG_LOG(GUI, "RpsQOutputTextEdit::display_output_object_occurrence start winob=" << _.winob
                << " of class:" << _.winob->compute_class(&_) << std::endl
                << "... dispob=" << _.dispob 	<< " of class:" << _.dispob->compute_class(&_) << std::endl
                << "... depth=" << depth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "display_output_object_occurrence")
                <<std::endl);
  std::lock_guard<std::recursive_mutex> guobwin (*(_.winob->objmtxptr()));
  auto qoutwpayl =
    _.winob->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT(qoutwpayl);
  RpsQOutputTextEdit* qoutxed = qoutwpayl->qtptr();
  RPS_ASSERT(qoutxed == this);
  RPS_DEBUG_LOG(GUI, "RpsQOutputTextEdit::display_output_object_occurrence dispob=" << _.dispob
                << " of class:" << _.dispob->compute_class(&_) << std::endl
                << ".. winob=" <<  _.winob
                << " of class:" << _.winob->compute_class(&_) << std::endl
                << ".. depth=" << depth);
  if (_.dispob.is_empty())
    {
      auto qcfmt = RpsQOutputTextEdit::empty_text_format();
      // we display a lozenge for empty objects
      auto qcursout = qoutxed->textCursor();
      qcursout.insertText(QString("◊"), //U+25CA LOZENGE
                          qcfmt);
    }
  else if (depth >  max_output_depth())
    {
      auto qcfmt = RpsQOutputTextEdit::etc_text_format();
      // we display a five dot
      auto qcursout = qoutxed->textCursor();
      qcursout.insertText(QString("⁙"), //U+2059 FIVE DOT PUNCTUATION
                          qcfmt);
    }
  else   // when _.dispob is not empty....
    {
      // Otherwise send RefPerSys selector display_object_occurrence_qt
      Rps_ObjectRef selob_display_object_occurrence_qt =
        RPS_ROOT_OB(_4ojpzRzyRWz02DNWMe);
      // of oid _4ojpzRzyRWz02DNWMe to winob, dispob, depth...
      RPS_DEBUG_LOG(GUI, "RpsQOutputTextEdit::display_output_object_occurrence winob=" << _.winob
                    << " of class:" << _.winob->compute_class(&_)
                    << " and payload-type:" << _.winob->payload_type_name()
                    << std::endl
                    << ".. dispob=" << _.dispob
                    << " of class:" << _.dispob->compute_class(&_) << std::endl
                    << ".. depth=" << depth << std::endl
                    << ".. selob_display_object_occurrence_qt=" << selob_display_object_occurrence_qt
                    << " of class:" << selob_display_object_occurrence_qt->compute_class(&_) << std::endl);
      Rps_TwoValues respair =
        Rps_ObjectValue(_.dispob).send2(&_, selob_display_object_occurrence_qt,
                                        _.winob, Rps_Value((intptr_t)depth));
      RPS_DEBUG_LOG(GUI, "RpsQOutputTextEdit::display_output_object_occurrence after send to dispob=" << _.dispob
                    << " of class:" << _.dispob->compute_class(&_) << std::endl
                    << ", with winob=" << _.winob
                    << " of class:" <<  _.winob->compute_class(&_) << std::endl
                    << ", respair .main=" << respair.main() << "+ .xtra=" << respair.xtra());
      if (!respair)
        throw RPS_RUNTIME_ERROR_OUT("display_output_object_occurrence failed winob="
                                    << _.winob
                                    << " dispob=" << _.dispob
                                    << " depth#" << depth);
    };				// end if _.dispob non-empty
  RPS_ASSERT(_.winob);
  RPS_DEBUG_LOG(GUI, "RpsQOutputTextEdit::display_output_object_occurrence end winob=" << _.winob
                << ", dispob=" << _.dispob
                << ", depth=" << depth);
} // end RpsQOutputTextEdit::display_output_object_occurrence


void
rps_display_output_object_occurrence(Rps_CallFrame*callerframe,
                                     Rps_ObjectRef argobwin, Rps_ObjectRef argobref, int depth)
{
  RPS_LOCALFRAME(rpskob_4ojpzRzyRWz02DNWMe, //display_object_occurrence_qt
                 //selector
                 callerframe, //
                 Rps_ObjectRef winob;
                 Rps_ObjectRef dispob;
                );
  _.winob = argobwin;
  _.dispob = argobref;
  RPS_ASSERT(_.winob);
  RPS_DEBUG_LOG(GUI, "rps_display_output_object_occurrence winob=" << _.winob
                << ", dispob=" << _.dispob
                << ", depth=" << depth
                << std::endl
                << "==== rps_display_output_object_occurrence backtrace ===" << std::endl
                <<  RPS_DEBUG_BACKTRACE_HERE(1, "rps_display_output_object_occurrence")
                << "**** end rps_display_output_object_occurrence simpleback ===="
                << std::endl);
  std::lock_guard<std::recursive_mutex> guobwin (*(_.winob->objmtxptr()));
  auto qoutwpayl =
    _.winob->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT(qoutwpayl);
  RpsQOutputTextEdit* qoutxed = qoutwpayl->qtptr();
  RPS_ASSERT(qoutxed);
  qoutxed->display_output_object_occurrence(&_, _.dispob, depth);
  RPS_DEBUG_LOG(GUI, "rps_display_output_object_occurrence ending winob=" << _.winob
                << ", dispob=" << _.dispob
                << ", depth=" << depth);
} // end rps_display_output_object_occurrence

//////////////////////////////////////////////////////////////// RpsQOutputTextDocument
RpsQOutputTextDocument::RpsQOutputTextDocument(RpsQWindow*parent)
  : QTextDocument(parent)
{
#warning incomplete RpsQOutputTextDocument
} // end RpsQOutputTextDocument::RpsQOutputTextDocument

RpsQOutputTextDocument::~RpsQOutputTextDocument()
{
} // end RpsQOutputTextDocument::~RpsQOutputTextDocument




////////////////////////////////////////////////////////////////
// C++ closure for _0TwK4TkhEGZ03oTa5m
//!display Val0 in Ob1Win at depth Val2Depth
extern "C" rps_applyingfun_t rpsapply_0TwK4TkhEGZ03oTa5m;
Rps_TwoValues
rpsapply_0TwK4TkhEGZ03oTa5m(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0val,
                            const Rps_Value arg1obwin, ///
                            const Rps_Value arg2depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_0TwK4TkhEGZ03oTa5m,
                 callerframe, //
                 Rps_Value val0v;
                 Rps_ObjectRef winob1;
                 Rps_Value depth2v;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  _.val0v = arg0val;
  _.winob1 = arg1obwin.to_object();
  _.depth2v = arg2depth;
  int depth = _.depth2v.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_0TwK4TkhEGZ03oTa5m start val0v=" << _.val0v
                << ", winob=" << _.winob1
                << ", depth=" << depth);
  ////==== body of _0TwK4TkhEGZ03oTa5m ====
  if (!_.winob1)
    throw RPS_RUNTIME_ERROR_OUT("display value " << _.val0v << " without window object");
  std::lock_guard<std::recursive_mutex> gu(*(_.winob1->objmtxptr()));
  auto winpayl = _.winob1->get_dynamic_payload<Rps_PayloadQt<QObject>>();
  RpsQOutputTextEdit* qouted= nullptr;
  if (!winpayl || !winpayl->qtptr()
      || !(qouted=qobject_cast<RpsQOutputTextEdit*>(winpayl->qtptr())))
    throw  RPS_RUNTIME_ERROR_OUT("display value " << _.val0v
                                 << " has bad window object " << _.winob1);
  /// we should display val0 in winob1 at depth2, using member function display_output_value
  qouted->display_output_value(&_, _.val0v, depth);
  if (_.val0v)
    _.resmainv = _.val0v;
  else
    _.resmainv = _.winob1;
  RPS_DEBUG_LOG(GUI, "rpsapply_0TwK4TkhEGZ03oTa5m end val0v=" << _.val0v
                << ", winob=" << _.winob1
                << ", depth=" << depth
                << ", resmain=" << _.resmainv);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _0TwK4TkhEGZ03oTa5m
} // end of rpsapply_0TwK4TkhEGZ03oTa5m !display Val0 in Ob1Win at depth Val2Depth


/// for the display_value_qt and display_object_occurrence_qt
/// RefPerSys methods see
/// https://gitlab.com/bstarynk/refpersys/-/wikis/output-subwindow-of-RefPerSys


////////////////////////////////////////////////////////////////
// C++ closure for _8KJHUldX8GJ03G5OWp
//!method int/display_value_qt
extern "C" rps_applyingfun_t rpsapply_8KJHUldX8GJ03G5OWp;
Rps_TwoValues
rpsapply_8KJHUldX8GJ03G5OWp(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0recv, ///
                            const Rps_Value arg1obwin, ///
                            const Rps_Value arg2depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_8KJHUldX8GJ03G5OWp,
                 callerframe, //
                 Rps_Value intv;
                 Rps_ObjectRef obwin;
                 Rps_Value depthv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  QTextCursor qcursout;
  QTextCharFormat qcfmt;
  ////==== body of _8KJHUldX8GJ03G5OWp ====
  _.intv = arg0recv;
  _.obwin = arg1obwin.as_object();
  _.depthv = arg2depth;
  RPS_DEBUG_LOG(GUI, "rpsapply_8KJHUldX8GJ03G5OWp start intv=" << _.intv
                << ", obwin=" << _.obwin
                << ", depth=" << _.depthv);
  RPS_ASSERT(_.obwin);
  RPS_ASSERT(_.depthv.is_int());
  qcfmt = RpsQOutputTextEdit::int_text_format();
  int depth = (int) (_.depthv.to_int());
  intptr_t curint = _.intv.to_int();
  std::lock_guard<std::recursive_mutex> guobwin (*(_.obwin->objmtxptr()));
  auto qoutwpayl =
    _.obwin->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT(qoutwpayl);
  RpsQOutputTextEdit* qoutxed = qoutwpayl->qtptr();
  RPS_ASSERT(qoutxed);
  /// FIXME: we have not decided yet (in commit 4b2038cfb35dafe) what
  /// cursor should be really used. Temporarily pretend it is the
  /// default one ...
  qcursout = qoutxed->textCursor();
  char intbuf[32];
  memset (intbuf, 0, sizeof(intbuf));
  snprintf(intbuf, sizeof(intbuf), "%lld", (long long) curint);
  qcursout.insertText(QString(intbuf), qcfmt);
  /// return reciever since success
  _.resmainv = _.intv;
  RPS_DEBUG_LOG(GUI, "rpsapply_8KJHUldX8GJ03G5OWp end intv=" << _.intv
                << ", obwin=" << _.obwin
                << ", depth=" << _.depthv
                << ", resmain=" << _.resmainv);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _8KJHUldX8GJ03G5OWp
} // end of rpsapply_8KJHUldX8GJ03G5OWp !method int/display_value_qt


////////////////////////////////////////////////////////////////

// C++ closure for _2KnFhlj8xW800kpgPt
//!method string/display_value_qt
extern "C" rps_applyingfun_t rpsapply_2KnFhlj8xW800kpgPt;
Rps_TwoValues
rpsapply_2KnFhlj8xW800kpgPt(Rps_CallFrame*callerframe,
                            const Rps_Value arg0_receiver,
                            const Rps_Value arg1_object_window,
                            const Rps_Value arg2_recursive_depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_2KnFhlj8xW800kpgPt,
                 callerframe, //
                 Rps_Value string_value;
                 Rps_ObjectRef object_window;
                 Rps_Value recursive_depth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );

  _.string_value = arg0_receiver;
  RPS_ASSERT(_.string_value.is_string());
  _.object_window = arg1_object_window.as_object();
  RPS_ASSERT(_.object_window);
  _.recursive_depth = arg2_recursive_depth;
  RPS_ASSERT(_.recursive_depth.is_int());
  RPS_DEBUG_LOG(GUI, "rpsapply_2KnFhlj8xW800kpgPt start string_value=" << _.string_value
                << "object_window, =" << _.object_window
                << ", recursive_depth=" <<  _.recursive_depth);
  ////==== body of _2KnFhlj8xW800kpgPt ====
  std::lock_guard<std::recursive_mutex> object_window_guard(*(_.object_window->objmtxptr()));
  auto qoutput_window_payload = _.object_window->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT(qoutput_window_payload);
  RpsQOutputTextEdit* qoutput_widget = qoutput_window_payload->qtptr();
  RPS_ASSERT(qoutput_widget);
  auto qcfmt = RpsQOutputTextEdit::string_text_format();
  auto qstr = QString(_.string_value.as_cstring());
#warning FIXME: we want to display nicely the non-printable characters such as tabulation, but how?
  auto qcursor = qoutput_widget->textCursor();
  qcursor.insertText(qstr, qcfmt);
  /// return reciever since success
  _.resmainv = _.string_value;
  RPS_DEBUG_LOG(GUI, "rpsapply_2KnFhlj8xW800kpgPt end string_value=" << _.string_value
                << "object_window, =" << _.object_window
                << ", recursive_depth=" <<  _.recursive_depth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _2KnFhlj8xW800kpgPt
} // end of rpsapply_2KnFhlj8xW800kpgPt !method string/display_value_qt


////////////////////////////////////////////////////////////////
// C++ closure for _7oa7eIzzcxv03TmmZH
//!method double/display_value_qt
extern "C" rps_applyingfun_t rpsapply_7oa7eIzzcxv03TmmZH;
Rps_TwoValues
rpsapply_7oa7eIzzcxv03TmmZH(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_7oa7eIzzcxv03TmmZH,
                 callerframe, //
                 Rps_Value doubleval;
                 Rps_ObjectRef object_window;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );

  ////==== body of _7oa7eIzzcxv03TmmZH !method double/display_value_qt ====
  _.doubleval = arg0_recv;
  RPS_ASSERT (_.doubleval.is_double());
  _.object_window = arg1_objwnd.as_object();
  RPS_ASSERT(_.object_window);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT(_.recdepth.is_int());
  RPS_DEBUG_LOG(GUI, "rpsapply_7oa7eIzzcxv03TmmZH start doubleval=" << _.doubleval
                << "object_window, =" << _.object_window
                << ", recdepth=" <<  _.recdepth);
  std::lock_guard<std::recursive_mutex> object_window_guard(*(_.object_window->objmtxptr()));
  auto qoutput_window_payload = _.object_window->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT(qoutput_window_payload);
  RpsQOutputTextEdit* qoutput_widget = qoutput_window_payload->qtptr();
  RPS_ASSERT(qoutput_widget);
  auto qcfmt = RpsQOutputTextEdit::double_text_format();
  char dblbuf[48];
  memset(dblbuf, 0, sizeof(dblbuf));
  snprintf(dblbuf, sizeof(dblbuf), "%g", _.doubleval.to_double());
  auto qstr = QString(dblbuf);
  auto qcursor = qoutput_widget->textCursor();
  qcursor.insertText(qstr, qcfmt);
  /// return reciever since success
  RPS_DEBUG_LOG(GUI, "rpsapply_7oa7eIzzcxv03TmmZH end doubleval=" << _.doubleval
                << "object_window, =" << _.object_window
                << ", recdepth=" <<  _.recdepth);
  _.resmainv = _.doubleval;
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _7oa7eIzzcxv03TmmZH
} // end of rpsapply_7oa7eIzzcxv03TmmZH !method double/display_value_qt




// C++ closure for _33DFyPOJxbF015ZYoi
//!method tuple/display_value_qt
extern "C" rps_applyingfun_t rpsapply_33DFyPOJxbF015ZYoi;
Rps_TwoValues
rpsapply_33DFyPOJxbF015ZYoi(Rps_CallFrame*callerframe, //
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_33DFyPOJxbF015ZYoi,
                 callerframe, //
                 Rps_Value tupleval;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value arg3v;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );
  ////==== body of _33DFyPOJxbF015ZYoi !method tuple/display_value_qt ====
  _.tupleval = arg0_recv;
  RPS_ASSERT (_.tupleval.is_tuple());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_33DFyPOJxbF015ZYoi start tupleval=" << _.tupleval
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  auto qoutwndload = _.objwnd->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT (qoutwndload);
  auto qoutwx = qoutwndload->qtptr();
  RPS_ASSERT (qoutwx);
  auto qcfmt = RpsQOutputTextEdit::tuple_text_format();
  auto qcursor = qoutwx->textCursor();
  unsigned cnt = _.tupleval.as_tuple()->cnt();
  qcursor.insertText("[", qcfmt);
  for (unsigned ix=0; ix<cnt; ix++)
    {
      if (ix>0)
        {
          qcursor.insertText(",", qcfmt);
          qoutwx->output_space_or_indented_newline(qcfmt, depthi);
        }
      rps_display_output_object_occurrence(callerframe, _.objwnd,
                                           _.tupleval.as_tuple()->at((int)ix),
                                           depthi+1);
    }
  qcursor.insertText("]", qcfmt);
  // success, so
  _.resmainv = _.tupleval;
  RPS_DEBUG_LOG(GUI, "rpsapply_33DFyPOJxbF015ZYoi end tupleval=" << _.tupleval
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _33DFyPOJxbF015ZYoi
} // end of rpsapply_33DFyPOJxbF015ZYoi !method tuple/display_value_qt


// C++ closure for _1568ZHTl0Pa00461I2
//!method set/display_value_qt
extern "C" rps_applyingfun_t rpsapply_1568ZHTl0Pa00461I2;
Rps_TwoValues
rpsapply_1568ZHTl0Pa00461I2(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_1568ZHTl0Pa00461I2,
                 callerframe, //
                 Rps_Value setval;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );

  ////==== body of _1568ZHTl0Pa00461I2 !method set/display_value_qt ====
  _.setval = arg0_recv;
  RPS_ASSERT (_.setval.is_set());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_1568ZHTl0Pa00461I2 start setval=" << _.setval
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  auto qoutwndload = _.objwnd->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT (qoutwndload);
  auto qoutwx = qoutwndload->qtptr();
  RPS_ASSERT (qoutwx);
  auto qcfmt = RpsQOutputTextEdit::set_text_format();
  auto qcursor = qoutwx->textCursor();
  unsigned cnt = _.setval.as_set()->cnt();
  qcursor.insertText("{", qcfmt);
  for (unsigned ix=0; ix<cnt; ix++)
    {
      if (ix>0)
        {
          qcursor.insertText(",", qcfmt);
          qoutwx->output_space_or_indented_newline(qcfmt, depthi);
        }
      rps_display_output_object_occurrence(callerframe, _.objwnd,
                                           _.setval.as_set()->at((int)ix),
                                           depthi+1);
    }
  qcursor.insertText("}", qcfmt);
  // success, so
  _.resmainv = _.setval;
  RPS_DEBUG_LOG(GUI, "rpsapply_1568ZHTl0Pa00461I2 end setval=" << _.setval
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _1568ZHTl0Pa00461I2
} // end of rpsapply_1568ZHTl0Pa00461I2 !method set/display_value_qt


// C++ closure for _18DO93843oX02UWzq6
//!method object/display_value_qt
extern "C" rps_applyingfun_t rpsapply_18DO93843oX02UWzq6;
Rps_TwoValues
rpsapply_18DO93843oX02UWzq6(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_objrecv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_18DO93843oX02UWzq6,
                 callerframe, //
                 Rps_ObjectRef obrecv;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _18DO93843oX02UWzq6 !method object/display_value_qt  ====
  _.obrecv = arg0_objrecv.as_object();
  RPS_ASSERT (_.obrecv);
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_18DO93843oX02UWzq6 start obrecv=" << _.obrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  std::lock_guard<std::recursive_mutex> obrecvdmtx(*(_.obrecv->objmtxptr()));
  auto qoutwndload = _.objwnd->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT (qoutwndload);
  auto qoutwx = qoutwndload->qtptr();
  RPS_ASSERT (qoutwx);
  qoutwx->display_output_object_occurrence(&_, _.obrecv, depthi);
  _.resmainv = _.obrecv;
#warning rpsapply_18DO93843oX02UWzq6 !method object/display_value_qt incomplete
  /* TODO: we probably want a more complex behavior, e.g. be able to
     hightlight all occurrences of the same object, have a menu or
     some way to "open" it, that is show its content, etc... This
     needs to be improved later! */
  RPS_DEBUG_LOG(GUI, "rpsapply_18DO93843oX02UWzq6 end obrecv=" << _.obrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _18DO93843oX02UWzq6
} // end of rpsapply_18DO93843oX02UWzq6 !method object/display_value_qt


// C++ closure for _0rgijx7CCnq041IZEd
//!method immutable_instance/display_value_qt
extern "C" rps_applyingfun_t rpsapply_0rgijx7CCnq041IZEd;
Rps_TwoValues
rpsapply_0rgijx7CCnq041IZEd (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0_inst, ///
                             const Rps_Value arg1_objwnd, ///
                             const Rps_Value arg2_recdepth, ///
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_0rgijx7CCnq041IZEd,
                 callerframe, //
                 Rps_InstanceValue instrecv;
                 Rps_ObjectRef objwnd;
                 Rps_ObjectRef obconn;
                 Rps_ObjectRef obattr;
                 Rps_Value recdepth;
                 Rps_Value compv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_qt====
  _.instrecv = Rps_InstanceValue(arg0_inst.as_instance());
  RPS_ASSERT (_.instrecv);
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_0rgijx7CCnq041IZEd start instrecv=" << _.instrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  auto qoutwndload = _.objwnd->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT (qoutwndload);
  auto qoutwx = qoutwndload->qtptr();
  RPS_ASSERT (qoutwx);
  auto qcfmt = RpsQOutputTextEdit::instance_text_format();
  auto qcursor = qoutwx->textCursor();
  _.obconn = _.instrecv->conn();
  RPS_ASSERT (_.obconn);
  /// we use the ☋ U+260B DESCENDING NODE sign, related to alchemical symbol for purify
  qcursor.insertText("☋", //U+260B DESCENDING NODE sign
                     qcfmt);
  qoutwx->display_output_object_occurrence(&_, _.obconn, depthi+1);
#warning rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_qt incomplete
  RPS_WARNOUT("rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_qt incomplete instrecv=" << _.instrecv);
  _.resmainv = _.instrecv;
  RPS_DEBUG_LOG(GUI, "rpsapply_0rgijx7CCnq041IZEd end instrecv=" << _.instrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _0rgijx7CCnq041IZEd
} // end of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_qt


// C++ closure for _6Wi00FwXYID00gl9Ma
//!method closure/display_value_qt
extern "C" rps_applyingfun_t rpsapply_6Wi00FwXYID00gl9Ma;
Rps_TwoValues
rpsapply_6Wi00FwXYID00gl9Ma (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0_clos, ///
                             const Rps_Value arg1_objwnd, ///
                             const Rps_Value arg2_recdepth, ///
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_6Wi00FwXYID00gl9Ma,
                 callerframe, //
                 Rps_ClosureValue closrecv;
                 Rps_ObjectRef objwnd;
                 Rps_ObjectRef obconn;
                 Rps_ObjectRef obmeta;
                 Rps_Value recdepth;
                 Rps_Value compv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _6Wi00FwXYID00gl9Ma !method closure/display_value_qt ====
  _.closrecv = Rps_ClosureValue(arg0_clos.as_closure());
  RPS_ASSERT (_.closrecv);
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_6Wi00FwXYID00gl9Ma start closrecv=" << _.closrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  _.obconn = _.closrecv->conn();
  unsigned width = _.closrecv->cnt();
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  auto qoutwndload = _.objwnd->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT (qoutwndload);
  auto qoutwx = qoutwndload->qtptr();
  RPS_ASSERT (qoutwx);
  auto qcfmt = RpsQOutputTextEdit::closure_text_format();
  auto qcursor = qoutwx->textCursor();
  _.obconn = _.closrecv->conn();
  RPS_ASSERT (_.obconn);
  /// we use the ⛝ U+26DD SQUARED SALTIRE, also meaning "closed exit"
  qcursor.insertText("⛝", //U+260B DESCENDING NODE sign
                     qcfmt);
  qoutwx->display_output_object_occurrence(&_, _.obconn, depthi+1);
  qoutwx->output_space_or_indented_newline(qcfmt, depthi+1);
  qcursor.insertText("❨", //U+2768 MEDIUM LEFT PARENTHESIS ORNAMENT
                     qcfmt);
  for (unsigned ix=0; ix<width; ix++)
    {
      if (ix>0)
        {
          qcursor.insertText(",", qcfmt);
          qoutwx->output_space_or_indented_newline(qcfmt, depthi+1);
        };
      _.compv = _.closrecv->at(ix);
      qoutwx->display_output_value(&_, _.compv, depthi+1);
      _.compv = nullptr;
    }
  qcursor.insertText("❩", //U+2769 MEDIUM RIGHT PARENTHESIS ORNAMENT
                     qcfmt);
  _.resmainv = _.closrecv;
  RPS_DEBUG_LOG(GUI, "rpsapply_6Wi00FwXYID00gl9Ma end closrecv=" << _.closrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _6Wi00FwXYID00gl9Ma
} // end of rpsapply_6Wi00FwXYID00gl9Ma !method closure/display_value_qt



// C++ closure for _52zVxP3mTue034OWsD
//!method qtptr/display_value_qt
extern "C" rps_applyingfun_t rpsapply_52zVxP3mTue034OWsD;
Rps_TwoValues
rpsapply_52zVxP3mTue034OWsD(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_qtptr,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth, ///
                            [[maybe_unused]] const Rps_Value arg3_, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_52zVxP3mTue034OWsD,
                 callerframe, //
                 Rps_Value qtrecv;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _52zVxP3mTue034OWsD !method qtptr/display_value_qt ====
  _.qtrecv = arg0_qtptr;
  RPS_ASSERT (_.qtrecv.is_qtptr());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_52zVxP3mTue034OWsD start qtrecv=" << _.qtrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  std::ostringstream outs;
  outs << _.qtrecv << std::flush;
  auto qoutwndload = _.objwnd->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT (qoutwndload);
  auto qoutwx = qoutwndload->qtptr();
  RPS_ASSERT (qoutwx);
  auto qcfmt = RpsQOutputTextEdit::qtptr_text_format();
  auto qcursor = qoutwx->textCursor();
  qcursor.insertText(outs.str().c_str(), qcfmt);
  _.resmainv = _.qtrecv;
  RPS_DEBUG_LOG(GUI, "rpsapply_52zVxP3mTue034OWsD end qtrecv=" << _.qtrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _52zVxP3mTue034OWsD
} // end of rpsapply_52zVxP3mTue034OWsD !method qtptr/display_value_qt


// C++ closure for _42cCN1FRQSS03bzbTz
//!method json/display_value_qt
extern "C" rps_applyingfun_t rpsapply_42cCN1FRQSS03bzbTz;
Rps_TwoValues
rpsapply_42cCN1FRQSS03bzbTz(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_json,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_42cCN1FRQSS03bzbTz,
                 callerframe, //
                 Rps_Value jsrecv;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );
  ////==== body of _42cCN1FRQSS03bzbTz !method json/display_value_qt ====
  ;
  _.jsrecv = arg0_json;
  RPS_ASSERT (_.jsrecv.is_json());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_42cCN1FRQSS03bzbTz start jsrecv=" << _.jsrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  std::ostringstream outs;
  outs << _.jsrecv << std::flush;
  auto qoutwndload = _.objwnd->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT (qoutwndload);
  auto qoutwx = qoutwndload->qtptr();
  RPS_ASSERT (qoutwx);
  auto qcfmt = RpsQOutputTextEdit::json_text_format();
  auto qcursor = qoutwx->textCursor();
  const char* pc = nullptr;
  const char* eol = nullptr;
  for (pc = outs.str().c_str(); pc && *pc; pc = (eol?(eol+1):nullptr))
    {
      eol = strchr(pc,  '\n');
      if (eol)
        {
          QString qspa(depthi, QChar(' '));
          qspa.append(std::string(pc, eol-pc-1).c_str());
          qcursor.insertText(qspa, qcfmt);
        }
      else
        {
          qcursor.insertText(pc, qcfmt);
        }
    }
  _.resmainv = _.jsrecv;
  RPS_DEBUG_LOG(GUI, "rpsapply_42cCN1FRQSS03bzbTz end jsrecv=" << _.jsrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _42cCN1FRQSS03bzbTz
} // end of rpsapply_42cCN1FRQSS03bzbTz !method json/display_value_qt




////////////////////////////////////////////////////////////////
// C++ closure for _4x9jd2yAe8A02SqKAx
//!method object/display_object_occurrence_qt
extern "C" rps_applyingfun_t rpsapply_4x9jd2yAe8A02SqKAx;
Rps_TwoValues
rpsapply_4x9jd2yAe8A02SqKAx (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0obj, //
                             const Rps_Value arg1obwin, ///
                             const Rps_Value arg2depth, //
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_ )
{
  RPS_LOCALFRAME(rpskob_4x9jd2yAe8A02SqKAx,
                 callerframe, //
                 Rps_ObjectRef recvob;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _4x9jd2yAe8A02SqKAx  !method object/display_object_occurrence_qt ====
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx start arg0obj=" << arg0obj
                << ", arg1obwin=" << arg1obwin
                << ", arg2depth=" << arg2depth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(2, "!method object/display_object_occurrence_qt"));
  _.recvob = arg0obj.as_object();
  RPS_ASSERT(_.recvob);
  _.objwnd = arg1obwin.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2depth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx recvob=" << _.recvob
                << " of class:" <<  _.recvob->compute_class(&_) << std::endl
                << "... objwnd=" << _.objwnd
                << " of class:" <<  _.objwnd->compute_class(&_)
                << "... depthi=" << depthi <<std::endl
                << "!method object/display_object_occurrence_qt" << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "rpsapply_4x9jd2yAe8A02SqKAx")
                <<std::endl);
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx objwnd=" << _.objwnd
                << " with payload@" << _.objwnd->get_payload()
                << " of type:" <<  _.objwnd->payload_type_name());
  std::lock_guard<std::recursive_mutex> objrecvmtx(*(_.recvob->objmtxptr()));
  auto qoutwpayl =  _.objwnd->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT(qoutwpayl);
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx objwnd=" << _.objwnd << std::endl
                << ".. qoutwpayl=" << qoutwpayl << '|' << qoutwpayl->payload_type_name() << std::endl
               );
  RpsQOutputTextEdit*outedit = qoutwpayl->qtptr();
  RPS_ASSERT(outedit);
  RpsQOutputTextDocument* outdoc = qobject_cast<RpsQOutputTextDocument*>(outedit->document());
  RPS_ASSERT(outdoc);
  QTextCursor qcurs(outdoc);
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx objwnd=" << _.objwnd
                << " outedit=" << outedit
                << " outdoc=" << outdoc
                << " recvob=" << _.recvob);
#warning should use symbol_text_format, oid_text_format, class_text_format ...
  if (auto symbpayl = _.recvob->get_dynamic_payload<Rps_PayloadSymbol>())
    {
      QTextCharFormat syf = outedit->symbol_text_format();
      RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx symbol recvob=" << _.recvob
                    << " named " << symbpayl->symbol_name());
      qcurs.insertText(QString(symbpayl->symbol_name().c_str()), syf);
    }
  else if (auto classpayl =  _.recvob->get_classinfo_payload())
    {
      QTextCharFormat cltxf = outedit->class_text_format();
      RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx class recvob=" << _.recvob
                    << " named " << classpayl->class_name_str());
      qcurs.insertText(QString(classpayl->class_name_str().c_str()), cltxf);
    }
  else
    {
      QTextCharFormat oidf = outedit->oid_text_format();
      RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx other recvob=" << _.recvob
                    << " of class " << _.recvob->compute_class(&_));
    }
#warning incomplete rpsapply_4x9jd2yAe8A02SqKAx !method object/display_object_occurrence_qt
  RPS_WARNOUT("incomplete rpsapply_4x9jd2yAe8A02SqKAx !method object/display_object_occurrence_qt" << std::endl
              << "... recvob=" << _.recvob
              << " objwnd=" << _.objwnd
              << " depthi=" << depthi << std::endl
              << RPS_FULL_BACKTRACE_HERE(2, "?£? rpsapply_4x9jd2yAe8A02SqKAx !method object/display_object_occurrence_qt")
              << std::endl);
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx end recvob=" << _.recvob
                << ", objwnd=" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _4x9jd2yAe8A02SqKAx
} // end of rpsapply_4x9jd2yAe8A02SqKAx !method object/display_object_occurrence_qt



// C++ closure for _5nSiRIxoYQp00MSnYA
//!method object!display_object_content_qt
extern "C" rps_applyingfun_t rpsapply_5nSiRIxoYQp00MSnYA;
Rps_TwoValues
rpsapply_5nSiRIxoYQp00MSnYA (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0obj, //
                             const Rps_Value arg1obwin, ///
                             const Rps_Value arg2depth, //
                             const Rps_Value arg3optqtposition, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_ )
{
  /* In the usual case, this RefPerSys method is called with 3
     arguments.  But in special cases, the 4th argument is a position
     in the document of the text cursor .... */
  RPS_LOCALFRAME(rpskob_5nSiRIxoYQp00MSnYA,
                 callerframe, //
                 Rps_ObjectRef recvob;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value optqtposition;
                 Rps_ObjectRef spacob;
                 Rps_Value setattrs;
                 Rps_ObjectRef attrob;
                 Rps_Value attrval;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _5nSiRIxoYQp00MSnYA !method object!display_object_content_qt ====
  _.recvob = arg0obj.as_object();
  RPS_ASSERT(_.recvob);
  _.objwnd = arg1obwin.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2depth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  _.optqtposition = arg3optqtposition;
  RPS_ASSERT (!_.optqtposition || _.optqtposition.is_int());
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  auto owinpayl =  _.objwnd->get_dynamic_payload<Rps_PayloadQt<RpsQWindow>>();
  RPS_DEBUG_LOG(GUI, "rpsapply_5nSiRIxoYQp00MSnYA start object!display_object_content_qt recvob=" << _.recvob
                << ", objwnd =" << _.objwnd
                << " of class:" <<  _.objwnd->compute_class(&_) << std::endl
                << "... depthi=" <<  depthi
                << ", owinpayl=" << owinpayl);
  std::lock_guard<std::recursive_mutex> objrecvmtx(*(_.recvob->objmtxptr()));
  RPS_ASSERT (owinpayl);
  RpsQWindow* qwindow = owinpayl->qtptr();
  RPS_ASSERT (qwindow);
  RpsQOutputTextEdit* qoutxtedit = qwindow->output_textedit();
  RpsQOutputTextDocument*qoutdoc = qwindow->output_doc();
  RPS_ASSERT(qoutxtedit);
  RPS_ASSERT(qoutdoc);
  auto qcursor = qoutxtedit->textCursor();
  if (_.optqtposition.is_int())
    {
      int qtpos = _.optqtposition.to_int();
      auto qothcursor = QTextCursor(qoutdoc);
      qothcursor.setPosition(qtpos,QTextCursor::MoveAnchor);
      qcursor = qothcursor;
    }
  /// see https://doc.qt.io/qt-5/richtext-advanced-processing.html
  {
    // the title block....
    qcursor.beginEditBlock();
    qcursor.insertFrame(qoutxtedit->objectcontent_frame_format());
    auto qcfmt = RpsQOutputTextEdit::objecttitle_text_format();
    qcursor.insertText("▣ ", //U+25A3 WHITE SQUARE CONTAINING BLACK SMALL SQUARE
                       qcfmt);
    qoutxtedit->display_output_object_occurrence(&_, _.recvob, depthi);
    qcursor.insertText(" ▤", //U+25A4 SQUARE WITH HORIZONTAL FILL
                       qcfmt);
    qcursor.insertText("\n");
    qcursor.insertText(_.recvob->string_oid().c_str(),
                       RpsQOutputTextEdit::oid_text_format());
    qcursor.endEditBlock();
  }
  {
    double mtim = _.recvob->get_mtime();
    RPS_DEBUG_LOG(GUI, "rpsapply_5nSiRIxoYQp00MSnYA recvob="
                  << _.recvob << ", mtim=" << mtim);
    time_t mtimt = (time_t) mtim;
    struct tm mtimtm = {};
    char timbuf[64];
    char timfrac[16];
    char timzon[16];
    memset (timbuf, 0, sizeof(timbuf));
    memset (timzon, 0, sizeof(timzon));
    memset (timfrac, 0, sizeof(timfrac));
    memset (&mtimtm, 0, sizeof(mtimtm));
    localtime_r (&mtimt, &mtimtm);
    strftime(timbuf, sizeof(timbuf), "%Y,%b %d %H:%M:%S", &mtimtm);
    strftime(timzon, sizeof(timzon), " %Z", &mtimtm);
    RPS_ASSERT(strlen(timbuf) + strlen(timzon) + 8 < sizeof(timbuf));
    double frtim = mtim - floor(mtim);
    if (frtim > 0.99)
      {
        strcpy(timfrac, ".99+");
      }
    else if (frtim >= 0.0)
      {
        snprintf(timfrac, sizeof(timfrac), "%.2f", frtim);
      }
    else
      strcpy(timfrac, ".??");
    strcat(timbuf, timfrac);
    strcat(timbuf, timzon);
    RPS_ASSERT(timbuf[sizeof(timbuf)-1] == (char)0);
    qcursor.insertText(QString("⌚ %1\n").arg(timbuf), //U+231A WATCH
                       RpsQOutputTextEdit::objectdecor_text_format());
  }
  _.setattrs = _.recvob->set_of_attributes(&_);
  RPS_DEBUG_LOG(GUI, "rpsapply_5nSiRIxoYQp00MSnYA recvob="
                << _.recvob << ", setattrs=" << _.setattrs);
  unsigned nbattrs = _.setattrs.as_set()->cardinal();
  if (nbattrs > 0)
    {
      qcursor.insertText(QString("%1 attributes\n").arg(nbattrs),
                         RpsQOutputTextEdit::objectdecor_text_format());
      for (unsigned aix=0; aix<nbattrs; aix++)
        {
          _.attrob = _.setattrs.as_set()->at(aix);
          _.attrval = _.recvob->get_attr1(&_, _.attrob);
          qcursor.insertText("● ", //U+25CF BLACK CIRCLE
                             RpsQOutputTextEdit::objectdecor_text_format());
          qoutxtedit->display_output_object_occurrence(&_, _.attrob, depthi+1);
          qcursor.insertText(" ➠ ", //U+27A0 HEAVY DASHED TRIANGLE-HEADED RIGHTWARDS ARROW
                             RpsQOutputTextEdit::objectdecor_text_format());
          qoutxtedit->display_output_value(&_, _.attrval, depthi+1);
          qcursor.insertText("\n");
        }
    }
  unsigned nbcomps = _.recvob->nb_components(&_);
#warning rpsapply_5nSiRIxoYQp00MSnYA !method object!display_object_content_qt incomplete
  RPS_WARNOUT("incomplete rpsapply_5nSiRIxoYQp00MSnYA !method object/display_object_content_qt" << std::endl
              << "... recvob=" << _.recvob
              << " objwnd=" << _.objwnd
              << " depthi=" << depthi
              << " optqtposition=" << _.optqtposition
              << " nbcomps=" << nbcomps);
  _.resmainv = _.recvob;
  RPS_DEBUG_LOG(GUI, "rpsapply_5nSiRIxoYQp00MSnYA end recvob=" << _.recvob
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _5nSiRIxoYQp00MSnYA
} // end of rpsapply_5nSiRIxoYQp00MSnYA !method object!display_object_content_qt


/************************************************************* end of file output_qrps.cc ****/
