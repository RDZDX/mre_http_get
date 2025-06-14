"C:\Program Files\MRE_SDK\tools\DllPackage.exe" "D:\MyGitHub\mre_http_get\mre_http_get.vcproj"
if %errorlevel% == 0 (
 echo postbuild OK.
  copy mre_http_get.vpp ..\..\..\MoDIS_VC9\WIN32FS\DRIVE_E\mre_http_get.vpp /y
exit 0
)else (
echo postbuild error
  echo error code: %errorlevel%
  exit 1
)

