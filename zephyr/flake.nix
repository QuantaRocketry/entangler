{
  description = "A nix shell for programming with zephyr.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

    # Customize the version of Zephyr used by the flake here
    zephyr.url = "github:zephyrproject-rtos/zephyr/v3.7.1";
    zephyr.flake = false;

    flake-utils.url = "github:numtide/flake-utils";

    zephyr-nix.url = "github:adisbladis/zephyr-nix";
    zephyr-nix.inputs.nixpkgs.follows = "nixpkgs";
    zephyr-nix.inputs.zephyr.follows = "zephyr";
  };

  outputs = { self, nixpkgs, flake-utils, ... }@inputs:
    (flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        inherit (nixpkgs) lib;

        callPackage = pkgs.newScope
          (pkgs // { zephyr = inputs.zephyr-nix.packages.${system}; });
      in { devShells.default = callPackage ./shell.nix { }; }));
}
