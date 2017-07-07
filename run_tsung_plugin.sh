sh ./clear_log.sh
rm -rf tsung_out
mkdir tsung_out
ulimit -n 1000000
tsung -f tsung_calm_plugin.xml start
sh ./get_log.sh
