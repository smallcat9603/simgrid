#! /bin/sh
#
# Simplistic script to rebuild our documentation with sphinx-build

set -e

if [ "x$1" != 'xdoxy' -a -e build/xml ] ; then
  echo "Doxygen not rerun: 'doxy' was not provided as an argument"
else
  rm -rf build/xml source/api/
  cd source; doxygen; cd ..
fi

if [ "x$1" != 'xjava' -a -e source/java ] ; then
  echo "javasphinx not rerun: 'java' was not provided as an argument"
else
  rm -rf source/java
  javasphinx-apidoc --force -o source/java/ ../src/bindings/java/org/simgrid/msg
  rm -f source/java/packages.rst # api_generated/source_java_packages.rst
  rm -f source/java/org/simgrid/msg/package-index.rst # api_generated/source_java_org_simgrid_msg_package-index.rst
  for f in source/java/org/simgrid/msg/* ; do
    # Add the package name to the page titles
    (echo -n "class org.simgrid.msg."; cat $f )>tmp
    mv tmp $f
    sed -i 's/==/========================/' $f # That's the right length knowing that I add 'class org.simgrid.msg.'
  done
#  sed -i 's/^.. java:type:: public class /.. java:type:: public class org.simgrid.msg/' source/java/org/simgrid/msg/*
  echo "javasphinx relaunched"
fi

sphinx-build -M html source build ${SPHINXOPTS}

set +x

cat source/img/graphical-toc.svg \
 | perl -pe 's/(xlink:href="http)/target="_top" $1/' \
 | perl -pe 's/(xlink:href=".*?.html)/target="_top" $1/' \
 > build/html/graphical-toc.svg

echo "List of missing references:"
for f in `(grep '<name>' build/xml/msg_8h.xml; \
           grep '<name>' build/xml/namespacesimgrid_1_1s4u.xml; \
 	   grep '<innerclass refid=' build/xml/namespacesimgrid_1_1s4u.xml ; \
          ) |sed 's/<[^>]*>//g'|sort` 
do

  if grep $f source/*rst | grep -q '.. doxygen[^::]*:: '"$f"'$' ||
     grep $f source/*rst | grep -q '.. doxygen[^::]*:: simgrid::[^:]*::[^:]*::'"$f"'$'  ; then :   
#    echo "$f documented"
  else 
    if grep -q $f ignored_symbols ; then : 
#      echo "$f ignored" # not documented
    else
      echo "$f"
    fi
  fi
done