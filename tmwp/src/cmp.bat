g++ -c *.cpp -I ..\include
del ..\lib\tmwp.lib
ar rcs ..\lib\tmwp.lib *.o