!include Library.nsh
!include x64.nsh 
!include LogicLib.nsh

!define PRODUCT_NAME "Tascher -Task Switcher-"
!define PRODUCT_VERSION "1.6.2.0"
!define PRODUCT_PUBLISHER "Frost Moon Project"
!define PRODUCT_WEB_SITE "http://www16.atpages.jp/rayna/index.html"

!define APPNAME "Tascher162"
!define APPDIR "files"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${APPNAME}_Setup.exe"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Japanese.nlf"


InstallDir "$PROGRAMFILES64\Tascher"

RequestExecutionLevel admin

ShowInstDetails show
ShowUninstDetails show

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

VIProductVersion ${PRODUCT_VERSION}
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME}"
VIAddVersionKey "LegalCopyright" "${PRODUCT_PUBLISHER}"
Icon "${NSISDIR}\Contrib\Graphics\Icons\classic-install.ico"

!define MUI_ABORTWARNING

SetCompressor /SOLID lzma
ShowInstDetails show
InstallColors D0D0D0 000000

Section
  SetOutPath "$INSTDIR"
  ${If} ${RunningX64}
    File ${APPDIR}\x64\MMHook.dll
    File ${APPDIR}\x64\Tascher.exe
  ${Else}
    File ${APPDIR}\MMHook.dll
    File ${APPDIR}\Tascher.exe
  ${EndIf}
  File ${APPDIR}\NYSL_withfaq.TXT
  File ${APPDIR}\Readme.txt
  File ${APPDIR}\Src.7z

  SetRegView 64

  !define RegistryKey "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tascher"
  WriteUninstaller $INSTDIR\Uninstall.exe
  ${If} ${RunningX64}
    WriteRegStr HKLM ${RegistryKey} "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION} x64"
  ${Else}
    WriteRegStr HKLM ${RegistryKey} "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
  ${EndIf}
  WriteRegStr HKLM ${RegistryKey} "DisplayIcon" $INSTDIR\Tascher.exe
  WriteRegStr HKLM ${RegistryKey} "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM ${RegistryKey} "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM ${RegistryKey} "DisplayVersion" "${PRODUCT_VERSION}"
SectionEnd

Section -Post
SectionEnd

Section "Uninstall"
  Delete $INSTDIR\MMHook.dll
  Delete $INSTDIR\Tascher.exe
  Delete $INSTDIR\NYSL_withfaq.TXT
  Delete $INSTDIR\Readme.txt
  Delete $INSTDIR\Src.7z
  Delete $INSTDIR\Uninstall.exe

  RMDir /r $INSTDIR

  SetRegView 64

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tascher"
SectionEnd

Function .onInstSuccess
   HideWindow
   ExecWait 'notepad "$INSTDIR\Readme.txt"'
FunctionEnd
