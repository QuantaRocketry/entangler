{ mkShell, zephyr, callPackage, pkgs, lib }:

mkShell {
  packages = with pkgs; [
    # (zephyr.sdk.override { targets = [ "arm-zephyr-eabi" ]; })
    zephyr.sdkFull
    zephyr.pythonEnv
    zephyr.hosttools-nix
    cmake
    ninja
    clang
    dfu-util
    picotool
    just
  ];
}
