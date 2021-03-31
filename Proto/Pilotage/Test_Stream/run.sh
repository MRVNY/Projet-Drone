[ $# = 0 ] && ../../../../../build.sh -p native -A Test_Stream
[ $(ls ../../../../../out/arsdk-native/staging/ | grep native-wrapper.sh | wc -l ) = 1 ] &&
../../../../../out/arsdk-native/staging/native-wrapper.sh ../../../../../out/arsdk-native/staging/usr/bin/Test_Stream ||
../../../../../out/arsdk-native/staging/native-darwin-wrapper.sh ../../../../../out/arsdk-native/staging/usr/bin/Test_Stream