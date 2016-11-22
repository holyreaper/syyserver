echo %cd%
cd %cd%  
rm -f *.cc *.h
protoc.exe --cpp_out .\  *.proto 
mv -f *.cc ..\..\event\
mv -f *.h ..\..\event\
