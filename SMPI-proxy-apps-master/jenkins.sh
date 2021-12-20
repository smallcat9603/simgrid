#! /bin/sh

export SIMGRID_PATH=/builds/simgrid_install
export CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE=1000000

set -exu
build_mode="$1"
case $build_mode in
  SMPI)
    ;;
  ModelChecker)
    ;;
  *)
    echo "Invalid build mode: ${build_mode}.  Either SMPI or ModelChecker".
    exit 1
esac
echo "Build mode $build_mode on $(uname -np)" >&2

rm -rf bin
mkdir -p bin
rm -rf Testing
for org in *.org ; do 
  emacs --batch --eval "(require 'org)" --eval '(org-babel-tangle-file "'${org}'")'
done

echo > bin/linkchecker.sh << EOF
lynx -hiddenlinks=merge -listonly -dump https://github.com/simgrid/SMPI-proxy-apps/ \
  | grep http | sed 's/ *[0-9]*. //' \
  | linkchecker --stdin --no-follow-url='http' --recursion-level=0
EOF

chmod +x bin/*.sh

cmake -Dbuild_mode=${build_mode} .

ctest -T test --output-on-failure --no-compress-output || true
if [ -f Testing/TAG ] ; then
   xsltproc $WORKSPACE/src/ctest2junit.xsl Testing/$( head -n 1 < Testing/TAG )/Test.xml > $WORKSPACE/CTestResults.xml
fi


