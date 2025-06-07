{
  inputs = {
    nixpkgs.url = github:NixOS/nixpkgs/nixos-unstable;

    graffiks.url = github:wentam/graffiks;
    graffiks.inputs.nixpkgs.follows = "nixpkgs";

    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {self, nixpkgs, flake-utils, ...}@inputs:
  flake-utils.lib.eachDefaultSystem (
    system:
    let
      pkgs = nixpkgs.legacyPackages.${system};
      bots = pkgs.rustPlatform.buildRustPackage {
        name = "bots";
        src = pkgs.fetchFromGitHub {
          owner = "retupmoca";
          repo = "bots";
          rev = "1b74cfe5f1a19a24fcde63abd78be25984e1f07e";
          hash = "sha256-FniE2sSaPRBuXkiohnPYU2WyD2reFQcJbsLWHbUORFI=";
        };
        buildInputs = with pkgs; [cargo rustc rust-cbindgen fmt glfw libpng];
        cargoHash = "sha256-0on1LOGK1Ldba7GnGd8+kjxNlb6zNOS/2/n2sdRtso8=";

        postInstall = ''
          export PATH=$PATH:${pkgs.rust-cbindgen}/bin/
          make
          mkdir -p $out/lib/ $out/include/bots/
          cp target/release/libbots.so $out/lib/
          cp bots.h $out/include/bots/
        '';
      };
    in rec {
      packages.default = pkgs.gcc12Stdenv.mkDerivation {
        name="GraffikalBots";
        src = ./.;
        buildInputs = with pkgs; [ bots cmake inputs.graffiks.packages.x86_64-linux.default SDL2 libGL xorg.libX11 ];
      };
    }
  );
}
