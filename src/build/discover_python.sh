#!/bin/sh

isKnownPython2_4() {

	# All supported debian machines currently run Python 2.4
	if [[ ! -f /etc/redhat-release ]]; then
		return 1
	fi

	# All CentOS is currently Python 2.4
	grep --quiet "CentOS" /etc/redhat-release
	isCentOS=$(( ! $? ))

	# Fedora 6 is Python 2.4
	grep --quiet "\(Zod\)" /etc/redhat-release
	isFedora6=$(( ! $? ))

	return $(( $isFedora6 || $isCentOS ))
}


isKnownPython2_4
isKnownOld=$?
if [[ $isKnownOld -eq 1 ]]; then
	case "$1" in

	  --libs)
		#python-config --libs
		#  -lpthread -ldl -lutil -lm -lpython2.5
		echo "-lpthread -ldl -lutil -lm -lpython2.4"
		;;

	  --includes)
		#python-config --includes
		#  -I/usr/include/python2.5 -I/usr/include/python2.5
		echo "-I/usr/include/python2.4 -I/usr/include/python2.4"
		;;

	  --cflags)
		#python-config --cflags
		#  -I/usr/include/python2.5 -I/usr/include/python2.5
		#  -fno-strict-aliasing -DNDEBUG -O2 -g -pipe -Wall
		#  -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector
		#  --param=ssp-buffer-size=4 -m32 -march=i386 -mtune=generic
		#  -fasynchronous-unwind-tables -D_GNU_SOURCE -fPIC
		#echo "-I/usr/include/python2.4 -I/usr/include/python2.4
		#  -fno-strict-aliasing -DNDEBUG -O2 -g -pipe -Wall
		#  -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector
		#  --param=ssp-buffer-size=4 -m32 -march=i386 -mtune=generic
		#  -fasynchronous-unwind-tables -D_GNU_SOURCE -fPIC
		;;

	  --ldflags)
		#python-config --ldflags
		#  -lpthread -ldl -lutil -lm -lpython2.5
		echo "-lpthread -ldl -lutil -lm -lpython2.4"
		;;
	esac
else
	PATH=$PATH:/usr/bin:/usr/local/bin
	export PATH
	PY_CONFIG=`which python-config`
	ret=$?
	if [ $ret != 0 ]; then
		echo "ERROR: Unable to locate 'python-config' in PATH=$PATH"
		exit $ret
	else
		$PY_CONFIG $1
	fi
fi

