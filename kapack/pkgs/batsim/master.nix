{ stdenv, fetchurl
, meson, ninja, pkgconfig
, simgrid, intervalset, boost, rapidjson, redox, hiredis, libev, zeromq, docopt_cpp, pugixml
, debug ? false
}:

stdenv.mkDerivation rec {
  pname = "batsim";
  version = "master";
  src = builtins.fetchurl "https://framagit.org/batsim/batsim/repository/master/archive.tar.gz";

  unpackPhase = ''
    # extract archive
    tar xf $src

    # as we suppose the archive has been obtained from gitlab on batsim's master branch,
    # the archive should contain a directory named "batsim-master-COMMIT".
    local parsed_commit=$(ls | sed -n -E 's/^${pname}-master-([[:xdigit:]]{40})$/\1/p')
    echo "git commit seems to be $parsed_commit (parsed from extracted archive directory name)"

    # hack meson's default version
    cd ${pname}-master-$parsed_commit
    local version_name="commit $parsed_commit (built by Nix from master branch)"
    echo "overriding meson's version: $version_name"
    sed -iE "s/version: '.*',/version: '$version_name',/" meson.build
  '';

  nativeBuildInputs = [
    meson
    ninja
    pkgconfig
  ];
  # runtimeDeps is used to generate multi-layered docker contained
  runtimeDeps = [
    simgrid
    intervalset
    rapidjson
    redox
    zeromq
    docopt_cpp
    pugixml
  ];
  buildInputs = [
    boost
  ] ++ runtimeDeps;

  mesonBuildType = if debug then "debug" else "release";
  enableParallelBuilding = true;

  meta = with stdenv.lib; {
    description = "An infrastructure simulator that focuses on resource management techniques.";
    homepage = "https://framagit.org/batsim/batsim";
    platforms = platforms.unix;
    license = licenses.lgpl3;
    broken = false;

    longDescription = ''
      Batsim is an infrastructure simulator that enables the study of resource management techniques.
    '';
  };
}
