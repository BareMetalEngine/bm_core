@ECHO OFF

REM ------------------------------------------------------------------------------

SETLOCAL

SET PLATFORM=windows
SET ONION="%~dp0\..\onion_tool\bin\onion.exe"

IF NOT EXIST %ONION% (
  git clone httsp://github.com/BareMetalEngine/onion .onion
  if ERRORLEVEL 1 (
	ECHO Unable to sync Onion repository
	EXIT /B 1
  )

  SET ONION=".onion/onion.exe"
  IF NOT EXIST %ONION% (
	ECHO Failed to find onion.exe in synced repository
	EXIT /B 1
  )
)

ECHO Using onion at '%ONION%', platform '%PLATFORM%'

%ONION% configure
if ERRORLEVEL 1 (
	ECHO Unable to configure test '%~1'
	POPD
	EXIT /B 1
)

%ONION% make
if ERRORLEVEL 1 (
	ECHO Unable to generate build files for test '%~1'
	POPD
	EXIT /B 1
)

EXIT /B 0

%ONION% build
if ERRORLEVEL 1 (
	ECHO Unable to compile test '%~1'
	POPD
	EXIT /B 1
)

%ONION% test
if ERRORLEVEL 1 (
	ECHO Failed to run tests for '%~1'
	POPD
	EXIT /B 1
)
