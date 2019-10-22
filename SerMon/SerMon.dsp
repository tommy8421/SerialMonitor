# Microsoft Developer Studio Project File - Name="SerMon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=SerMon - Win32 fre
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SerMon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SerMon.mak" CFG="SerMon - Win32 fre"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SerMon - Win32 fre" (based on "Win32 (x86) External Target")
!MESSAGE "SerMon - Win64 fre" (based on "Win32 (x86) External Target")
!MESSAGE "SerMon - Win32 chk" (based on "Win32 (x86) External Target")
!MESSAGE "SerMon - Win64 chk" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "SerMon - Win32 fre"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x86"
# PROP BASE Intermediate_Dir "x86"
# PROP BASE Cmd_Line "BuildDrv.bat -W7XP fre"
# PROP BASE Rebuild_Opt "BuildDrv.bat -W7XP fre"
# PROP BASE Bsc_Name "SerMon.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x86"
# PROP Intermediate_Dir "x86"
# PROP Cmd_Line "BuildDrv.bat -W7XP fre"
# PROP Rebuild_Opt "BuildDrv.bat -W7XP fre "
# PROP Target_File "PdaAux.sys"
# PROP Bsc_Name "SerMon.bsc"
# PROP Target_Dir ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!ELSEIF  "$(CFG)" == "SerMon - Win64 fre"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x64"
# PROP BASE Intermediate_Dir "x64"
# PROP BASE Cmd_Line "BuildDrv.bat -W7X64 fre"
# PROP BASE Rebuild_Opt "BuildDrv.bat -W7X64 fre"
# PROP BASE Bsc_Name "SerMon.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x64"
# PROP Intermediate_Dir "x64"
# PROP Cmd_Line "BuildDrv.bat -W7X64 fre"
# PROP Rebuild_Opt "BuildDrv.bat -W7X64 fre"
# PROP Target_File "PdaAux64.sys"
# PROP Bsc_Name "SerMon.bsc"
# PROP Target_Dir ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!ELSEIF  "$(CFG)" == "SerMon - Win32 chk"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x86"
# PROP BASE Intermediate_Dir "x86"
# PROP BASE Cmd_Line "BuildDrv.bat -W7XP chk"
# PROP BASE Rebuild_Opt "BuildDrv.bat -W7XP chk"
# PROP BASE Target_File "PdaAux.sys"
# PROP BASE Bsc_Name "SerMon.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x86"
# PROP Intermediate_Dir "x86"
# PROP Cmd_Line "BuildDrv.bat -W7XP chk"
# PROP Rebuild_Opt "BuildDrv.bat -W7XP chk"
# PROP Target_File "PdaAux.sys"
# PROP Bsc_Name "SerMon.bsc"
# PROP Target_Dir ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!ELSEIF  "$(CFG)" == "SerMon - Win64 chk"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x64"
# PROP BASE Intermediate_Dir "x64"
# PROP BASE Cmd_Line "BuildDrv.bat -W7X64 chk"
# PROP BASE Rebuild_Opt "BuildDrv.bat -W7X64 chk"
# PROP BASE Target_File "PdaAux.sys"
# PROP BASE Bsc_Name "SerMon.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x64"
# PROP Intermediate_Dir "x64"
# PROP Cmd_Line "BuildDrv.bat -W7X64 chk"
# PROP Rebuild_Opt "BuildDrv.bat -W7X64 chk"
# PROP Target_File "PdaAux64.sys"
# PROP Bsc_Name "SerMon.bsc"
# PROP Target_Dir ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!ENDIF 

# Begin Target

# Name "SerMon - Win32 fre"
# Name "SerMon - Win64 fre"
# Name "SerMon - Win32 chk"
# Name "SerMon - Win64 chk"

!IF  "$(CFG)" == "SerMon - Win32 fre"

!ELSEIF  "$(CFG)" == "SerMon - Win64 fre"

!ELSEIF  "$(CFG)" == "SerMon - Win32 chk"

!ELSEIF  "$(CFG)" == "SerMon - Win64 chk"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DevExt.cpp
# End Source File
# Begin Source File

SOURCE=.\DrvClass.cpp
# End Source File
# Begin Source File

SOURCE=.\SerMon.cpp
# End Source File
# Begin Source File

SOURCE=.\SerMon.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DevExt.h
# End Source File
# Begin Source File

SOURCE=.\DrvClass.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SerMon.h
# End Source File
# Begin Source File

SOURCE=.\SerMonEx.h
# End Source File
# End Group
# End Target
# End Project
