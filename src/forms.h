#ifndef h_forms_h
#define h_forms_h

#include "sections.h"

/* Form event handlers. Starter.c installs these with FrmSetEventHandler, so
 * they must be declared with their code section (see sections.h). The map
 * and note forms are declared in map.h and note.h.
 */
Boolean MainFormHandleEvent ( EventPtr eventP ) SEC_KRONOS;

void PicFormInit ( FormPtr frmP ) SEC_KRONOS;
void PicFormExit ( FormPtr frmP ) SEC_KRONOS;
Boolean PicFormHandleEvent ( EventPtr eventP ) SEC_KRONOS;

void PickerFormInit ( FormPtr frmP ) SEC_KRONOS;
void PickerFormExit ( FormPtr frmP ) SEC_KRONOS;
Boolean PickerFormHandleEvent ( EventPtr eventP ) SEC_KRONOS;

#endif
