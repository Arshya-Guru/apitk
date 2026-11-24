@echo off
rem Launch apitk (custom ITK-SNAP build) from the folder this script lives in.
rem Put this folder on PATH to run "apitk" from any terminal.
"%~dp0ITK-SNAP.exe" --console %*
