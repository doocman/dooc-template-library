cd $(dirname -- "$0")
FILES=$(find lib -iname '*.hpp' -or -iname '*.cpp' -or -iname '*.cppm')
$CLANG_FORMAT --style=file -i $FILES
