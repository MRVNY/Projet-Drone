cd Proto/Pilotage
[ $# = 1 ] && cd $(ls | grep $1)
[ $(ls | grep run.sh | wc -l) = 1 ] && bash run.sh