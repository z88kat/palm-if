
#ifndef h_note_h
#define h_note_h

#include "sections.h"

void NoteFormInit ( FormPtr frmP ) SEC_KRONOS;
void NoteFormExit ( FormPtr frmP ) SEC_KRONOS;
Boolean NoteFormHandleEvent ( EventPtr eventP ) SEC_KRONOS;

void update_scrollbar ( FormPtr f, UInt16 fld, UInt16 scrollbar ) SEC_KRONOS;
void scroll_note ( FormPtr f, int num, Boolean redraw, UInt16 fld ) SEC_KRONOS;
void scroll_page ( FormPtr f, WinDirectionType direction, UInt16 fld ) SEC_KRONOS;

#endif
