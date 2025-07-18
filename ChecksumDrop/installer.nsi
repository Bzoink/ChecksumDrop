; Publish with command:
; dotnet publish ChecksumDrop.csproj -c Release -r win-x64 --self-contained false -o ./publish
; installer.nsi
!define APP_NAME "ChecksumDrop"
!define APP_EXE "ChecksumDrop.exe"
!define COMPANY_NAME "The fluffy cloud corporation"
!define VERSION "1.0"

; Installer settings
Name "${APP_NAME}"
OutFile "installer\ChecksumDrop_${VERSION}.exe"
InstallDir "$LOCALAPPDATA\${APP_NAME}"
RequestExecutionLevel user
Unicode True
Icon "ChecksumDrop.ico"
SetCompressor /SOLID lzma

; Show installation details
ShowInstDetails show

; Pages
Page directory
Page instfiles

; Install section
Section "Install"
  DetailPrint "Starting installation..."
  SetOutPath "$INSTDIR"
  DetailPrint "Copying files to $INSTDIR..."
  File /r "publish\*.*" ; Include all files from the publish folder
  File "ChecksumDrop.ico"
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Write registry entries
  WriteRegStr HKCU "Software\${COMPANY_NAME}\${APP_NAME}" "InstallPath" "$INSTDIR"
  WriteRegStr HKCU "Software\${COMPANY_NAME}\${APP_NAME}" "Version" "${VERSION}"

  ; Create shortcut in Start Menu
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\ChecksumDrop.ico"
  CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"

  ; Create shortcut on Desktop
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\ChecksumDrop.ico"
SectionEnd

; Uninstall section
Section "Uninstall"
  DetailPrint "Starting uninstallation..."
  IfFileExists "$INSTDIR\*.*" 0 +2
  Delete "$INSTDIR\*.*"
  RMDir "$INSTDIR"
  Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
  RMDir "$SMPROGRAMS\${APP_NAME}"
  DeleteRegKey HKCU "Software\${COMPANY_NAME}\${APP_NAME}"
  DetailPrint "Uninstallation completed."
SectionEnd
