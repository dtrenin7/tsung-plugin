#tar -xvf tsung_plugin.tar
#cd tsung_plugin
rm -rf build
mkdir build
g++ -std=c++11 -Wall -fPIC -Wno-switch -Wno-return-type -shared -o build/csv_reader_nif.so -I/lib64/erlang/erts-5.10.4/include  main.cpp svp_error.cpp svp_ini.cpp svp_settings.cpp svp_utils.cpp
erl -compile csv_reader_nif.erl
mv csv_reader_nif.beam build/csv_reader_nif.beam
cp -v build/csv_reader_nif.so /usr/bin/
cp -v build/csv_reader_nif.beam /lib/erlang/lib/tsung-1.6.0/ebin/
cp -v csv_reader_nif.ini /etc/
