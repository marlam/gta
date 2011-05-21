; Copyright (C) 2010, 2011
; Martin Lambers <marlam@marlam.de>
;
; Copying and distribution of this file, with or without modification, are
; permitted in any medium without royalty provided the copyright notice and this
; notice are preserved. This file is offered as-is, without any warranty.

!include "MUI.nsh"
!include "AddToPath.nsh"

; The name of the installer
Name "GTATool ${PACKAGE_VERSION}"

; The file to write
OutFile "gtatool-${PACKAGE_VERSION}-w32.exe"

; Require Administrator privileges
RequestExecutionLevel admin

; The default installation directory
InstallDir "$PROGRAMFILES\gtatool-${PACKAGE_VERSION}"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\gtatool-${PACKAGE_VERSION}" "Install_Dir"

SetCompressor lzma
ShowInstDetails show

Var MUI_TEMP
Var STARTMENU_FOLDER

; Interface Settings
  !define MUI_ABORTWARNING
  !define MUI_ICON appicon.ico
  ;!define MUI_UNICON appicon.ico
 
; Installer Pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_LICENSE "notes.txt"
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

; Languages: native first, others sorted by the language code (cs, de, ...)
  !insertmacro MUI_LANGUAGE "English"

; Program file installation
Section "GTATool Program" SecTools
  SetOutPath $INSTDIR\bin
  FILE gta.exe
SectionEnd

; Last section is a hidden one.
Var MYTMP
Section
  SetShellVarContext all
  WriteUninstaller "$INSTDIR\uninstall.exe"
  ; Write the installation path into the registry
  WriteRegStr HKLM "Software\gtatool-${PACKAGE_VERSION}" "Install_Dir" "$INSTDIR"
  ; Windows Add/Remove Programs support
  StrCpy $MYTMP "Software\Microsoft\Windows\CurrentVersion\Uninstall\gtatool-${PACKAGE_VERSION}"
  WriteRegStr       HKLM $MYTMP "DisplayName"     "GTATool ${PACKAGE_VERSION}"
  WriteRegExpandStr HKLM $MYTMP "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegExpandStr HKLM $MYTMP "InstallLocation" "$INSTDIR"
  WriteRegStr       HKLM $MYTMP "DisplayVersion"  "${PACKAGE_VERSION}"
  WriteRegStr       HKLM $MYTMP "Publisher"       "The GTATool developers"
  WriteRegStr       HKLM $MYTMP "URLInfoAbout"    "http://gta.nongnu.org/"
  WriteRegStr       HKLM $MYTMP "HelpLink"        "http://gta.nongnu.org/"
  WriteRegStr       HKLM $MYTMP "URLUpdateInfo"   "http://gta.nongnu.org/"
  WriteRegDWORD     HKLM $MYTMP "NoModify"        "1"
  WriteRegDWORD     HKLM $MYTMP "NoRepair"        "1"
  ; Start menu
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\GTATool.lnk" "$INSTDIR\bin\gta.exe" "gui"
  WriteINIStr "$SMPROGRAMS\$STARTMENU_FOLDER\Website.url" "InternetShortcut" "URL" "http://gta.nongnu.org/"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  !insertmacro MUI_STARTMENU_WRITE_END
  ; Add bin directory to the PATH
  Push $INSTDIR\bin
  Call AddToPath
SectionEnd

; Uninstaller
Section "Uninstall"
  SetShellVarContext all
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\gtatool-${PACKAGE_VERSION}"
  DeleteRegKey HKLM "Software\gtatool-${PACKAGE_VERSION}"
  DeleteRegKey /ifempty HKCU "Software\gtatool-${PACKAGE_VERSION}"
  ; Remove start menu entries
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
  RMDir /r "$SMPROGRAMS\$MUI_TEMP"
  ; Remove files
  RMDir /r $INSTDIR
  ; Remove bin directory from the PATH
  Push $INSTDIR\bin
  Call un.RemoveFromPath
SectionEnd
