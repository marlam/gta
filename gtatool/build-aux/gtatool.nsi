; Copyright (C) 2010 Martin Lambers

!include "MUI.nsh"
!include "AddToPath.nsh"

; The name of the installer
Name "gtatool ${PACKAGE_VERSION}"

; The file to write
OutFile "gtatool-${PACKAGE_VERSION}.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\gtatool-${PACKAGE_VERSION}"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\gtatool-${PACKAGE_VERSION}" "Install_Dir"

;SetCompressor lzma
ShowInstDetails show

Var MUI_TEMP
Var STARTMENU_FOLDER

;Interface Settings

  !define MUI_ABORTWARNING
  !define MUI_ICON appicon.ico
  ;!define MUI_UNICON appicon.ico
 
; Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_DIRECTORY
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\gtatool-${PACKAGE_VERSION}"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;Languages
 
  !insertmacro MUI_LANGUAGE "English"

Section "Gtatool Program" SecTools

  SetOutPath $INSTDIR\bin
  FILE gta.exe
  
SectionEnd

Var MYTMP

# Last section is a hidden one.
Section
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Write the installation path into the registry
  WriteRegStr HKLM "Software\gtatool-${PACKAGE_VERSION}" "Install_Dir" "$INSTDIR"

  # Windows Add/Remove Programs support
  StrCpy $MYTMP "Software\Microsoft\Windows\CurrentVersion\Uninstall\gtatool-${PACKAGE_VERSION}"
  WriteRegStr       HKLM $MYTMP "DisplayName"     "gtatool ${PACKAGE_VERSION}"
  WriteRegExpandStr HKLM $MYTMP "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegExpandStr HKLM $MYTMP "InstallLocation" "$INSTDIR"
  WriteRegStr       HKLM $MYTMP "DisplayVersion"  "${PACKAGE_VERSION}"
  WriteRegStr       HKLM $MYTMP "Publisher"       "Martin Lambers"
  WriteRegStr       HKLM $MYTMP "URLInfoAbout"    "http://www.nongnu.org/gta/"
  WriteRegStr       HKLM $MYTMP "HelpLink"        "http://www.nongnu.org/gta/"
  WriteRegStr       HKLM $MYTMP "URLUpdateInfo"   "http://www.nongnu.org/gta/"
  WriteRegDWORD     HKLM $MYTMP "NoModify"        "1"
  WriteRegDWORD     HKLM $MYTMP "NoRepair"        "1"

!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
;Create shortcuts
  CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\GTA gui.lnk" "$INSTDIR\bin\gta.exe" "gui"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  WriteINIStr "$SMPROGRAMS\$STARTMENU_FOLDER\Website.url" "InternetShortcut" "URL" "http://www.nongnu.org/gta/"
!insertmacro MUI_STARTMENU_WRITE_END

; Add bin directory to the path
  Push $INSTDIR\bin
  Call AddToPath

SectionEnd

;Descriptions

  LangString DESC_SecTools ${LANG_ENGLISH} "Gtatool Program"

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecTools}     $(DESC_SecTools)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\gtatool-${PACKAGE_VERSION}"
  DeleteRegKey HKLM "Software\gtatool-${PACKAGE_VERSION}"
  ; Remove files
  Delete $INSTDIR\bin\gta.exe
  RMDir $INSTDIR\bin
  ; Remove uninstaller
  Delete $INSTDIR\uninstall.exe
  ; Remove shortcuts, if any
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
  Delete "$SMPROGRAMS\$MUI_TEMP\GTA gui.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Website.url"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
  startMenuDeleteLoop:
    ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    IfErrors startMenuDeleteLoopDone
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:
  DeleteRegKey /ifempty HKCU "Software\gtatool-${PACKAGE_VERSION}"
  ; Remove directories used
  RMDir "$INSTDIR"
  Push $INSTDIR\bin
  Call un.RemoveFromPath

SectionEnd
