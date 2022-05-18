DIR="/lib/modules/`uname -r`/extra2"
[ ! -d "$DIR" ] && mkdir $DIR

cp $1 $DIR