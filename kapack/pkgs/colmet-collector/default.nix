{ stdenv, pkgs, fetchFromGitHub, python37Packages }:

python37Packages.buildPythonApplication rec {
  name = "colmet-collector-${version}";
  version = "0.0.1";

  src = fetchFromGitHub {
    owner = "oar-team";
    repo = "colmet-collector";
    rev = "a7a5e4460950c4808c1cbd0cf59c8808c52012e6";
    sha256 = "1ikqmym68pkmgxwknhmq42bsn5yn0yvpj5wm0wg0q85i9jdlgwwz";
  };

  propagatedBuildInputs = with python37Packages; [
    pyzmq
    requests
  ];

  # Tests do not pass
  doCheck = false;

  meta = with stdenv.lib; {
    description = "Metrics collector for Rust version of Colmet (colmet-rs)";
    homepage    = https://github.com/oar-team/colmet-collector;
    platforms   = platforms.unix;
    licence     = licenses.gpl2;
  };
}
