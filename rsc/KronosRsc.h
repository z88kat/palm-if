/*
 * Kronos resource IDs, shared by kronos.rcp and the C sources.
 */

#ifndef h_kronos_rsc_h
#define h_kronos_rsc_h


#define MainForm                             1000
#define MainStartButton                      1001
#define MainPicButton                        1006
#define MainTinyCheckbox                     1019
#define MainMediumCheckbox                   1020
#define MainNormalCheckbox                   1021
#define MainFullscreenCheckbox               1024
#define MainEntryField                       1002
#define MainPromptLabel                      1003
#define MainPopList                          1018
#define MainScrollUpRepeating                1022
#define MainScrollDownRepeating              1023
#define MainCRLFGraphicButton                1004
#define MainClearGraphicButton               1005
#define MainNGraphicButton                   1007
#define MainSGraphicButton                   1008
#define MainEGraphicButton                   1009
#define MainWGraphicButton                   1010
#define MainIGraphicButton                   1011
#define MainNavigateGraphicButton            1012
#define MainWordsGraphicButton               1013
#define MainNil3GraphicButton                1014
#define MainLoadGraphicButton                1015
#define MainSaveGraphicButton                1016
#define MainLGraphicButton                   1017
#define MainGroupID                          1

#define AboutForm                            1100
#define AboutOKButton                        1105
#define AboutVersionField                    1106
#define AboutDetailsField                    1107
#define AboutUnnamed1101Label                1101
#define AboutText1Label                      1103
#define AboutUnnamed1104Label                1104
#define AboutUnnamed1108Label                1108
#define AboutUnnamed1109Label                1109

#define PicForm                              1200
#define PicOKButton                          1201

#define PickerForm                           1300
#define PickerPlayButton                     1311
#define PickerUnnamed1301Label               1301
#define PickerGamesList                      1310

#define MoreForm                             1400
#define MoreUnnamed1401Button                1401

#define MapForm                              1500
#define MapOKButton                          1501
#define MapSouthButton                       1508
#define MapNorthButton                       1509
#define MapRecenterButton                    1511
#define MapWestButton                        1512
#define MapEastButton                        1513
#define MapNorthwestButton                   1516
#define MapNortheastButton                   1517
#define MapSoutheastButton                   1518
#define MapSouthwestButton                   1519
#define MapUpButton                          1522
#define MapDownButton                        1523
#define MapWestField                         1502
#define MapEastField                         1503
#define MapSouthField                        1504
#define MapNorthwestField                    1505
#define MapNortheastField                    1506
#define MapNorthField                        1507
#define MapCenterField                       1510
#define MapSoutheastField                    1514
#define MapSouthwestField                    1515
#define MapFindList                          1521
#define MapFindPopTrigger                    1520

#define NoteForm                             1600
#define NoteOKButton                         1601
#define NoteNoteField                        1602
#define NoteUnnamed1603Label                 1603
#define NoteScrollyScrollBar                 1604

#define RomIncompatibleAlert                 1001
#define RomIncompatibleOK                    0

#define MessageAlert                         1100
#define MessageOK                            0

#define YesNoAlert                           1200
#define YesNoYes                             0
#define YesNoNo                              1

#define MainFormMenuBar                      1000
#define MainOptionsMenu                      1000
#define MainOptionsQuitKronos                1000
#define MainOptionsAboutKronos               1001
#define EditMenu                             1100
#define EditUndo                             1100
#define EditCut                              1101
#define EditCopy                             1102
#define EditPaste                            1103
#define EditSelectAll                        1104
#define EditKeyboard                         1106
#define EditGrafitti                         1107
#define EditFreeHeapRAM                      1109
#define GameMenu                             1200
#define GameLoad                             1200
#define GameSave                             1201
#define GameZoomPicture                      1203
#define GameAutomapExits                     1205
#define GameAutomapView                      1206
#define GameAddGameNotes                     1208
#define GameQuit                             1210
#define ColourMenu                           1300
#define ColourAtariST                        1300
#define ColourBlackOnWhite                   1301
#define ColourCommodoreAmiga                 1302
#define ColourCommodoreVic20                 1303
#define ColourMSDOS                          1304
#define Bitmap2                              1002
#define Bitmap3                              1008
#define Bitmap4                              1011
#define Bitmap5                              1012
#define Bitmap6                              1018

/* bitmap families (Tbmp) */
#define WBitmapFamily                        2003
#define EBitmapFamily                        2002
#define SBitmapFamily                        2001
#define NBitmapFamily                        2000
#define IBitmapFamily                        2004
#define NilBitmapFamily                      2005
#define LBitmapFamily                        2006
#define GoBitmapFamily                       2007
#define ClearBitmapFamily                    2008
#define LoadBitmapFamily                     2009
#define SaveBitmapFamily                     2010
#define WordsBitmapFamily                    2011
#define ReturnCharBitmapFamily               2012
#define NavigateBitmapFamily                 2013
#define Bat1BitmapFamily                     2014
#define Bat2BitmapFamily                     2015
#define Bat3BitmapFamily                     2016
#define Bat4BitmapFamily                     2017
#define ThinkingBitmapFamily                 2018

#endif
