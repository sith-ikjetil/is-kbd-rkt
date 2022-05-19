DIR="/lib/modules/`uname -r`/extra"
[ ! -d "$DIR" ] && mkdir $DIR

cp $1 $DIR
