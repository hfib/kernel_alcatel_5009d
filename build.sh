# User stuff
VERSION=1.1
TYPE=zImage

# Kbuild
export KBUILD_BUILD_USER=Hfib
export KBUILD_BUILD_HOST=4pda

# Make
export ARCH=arm ARCH_MTK_PLATFORM=mt6580
export PATH=`pwd`/../prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin:$PATH
CONFIG=lcsh6580_weg_sz_n

# Zipping
USEZIP=1
IMAGE=out/arch/arm/boot/$TYPE

if [[ -z `which zip` ]]; then
  echo "Warning: zip utility isn't found, no ability to make installer"
  USEZIP=
fi

USAGE() {
  echo "Usage: $0 instruction"
  echo "Instructions:"
  echo "  autoall - Build without prompts"
  echo "  all     - Build with prompts"
  echo "  config  - Configure only"
  echo "  build   - Build only"
  echo "  zip     - Zip only"
  echo "  clean   - Clean repository"
  echo "  help    - Show usage"
  echo "  mhelp - Run 'make help'"
  exit 0
}

HELP() {
  make help
  exit 0
}

AUTO() {
  : ${FORCE_RECONFIG:=y}
  : ${FORCE_REBUILD:=y}
}

CONFIG() {
  make O=out "$CONFIG"_defconfig
  if [[ $? == 0 ]]; then
    echo "Config is done"
  else
    echo "Config is failed"
    exit 2
  fi
}

BUILD() {
  if [[ ! -f "out/.config" ]]; then
    echo "Do config first"
    exit 2
  fi
  make O=out $TYPE
  if [[ $? == 0 ]]; then
    echo "Kernel is built"
  else
    echo "Building is failed"
    exit 2
  fi
}

ZIP() {
  NAME=andromeda-v$VERSION.zip

  if [[ ! -f "$IMAGE" ]]; then
    echo "Do image first"
    exit 2
  fi
  rm -f $NAME

  cp $IMAGE AnyKernel3
  cd AnyKernel3
  zip -r ../$NAME *
  cd ..

  echo "Installer is done"
}

CLEAN() {
  # make clean && make mrproper won't work, smh
  git clean -Xdf
}

check() {
  if [[ "${!1}" == y ]] || [[ "${!1}" == n ]]; then
    return 0
  else
    return 1
  fi
}

mkdir out -p

case "$1" in
  "autobuild" ) AUTO ;;
        "all" )      ;;
     "config" ) CONFIG; exit 0;;
      "build" )  BUILD; exit 0;;
        "zip" )    ZIP; exit 0;;
      "clean" )  CLEAN; exit 0;;
   "makehelp" ) HELP ;;
    "help"|"" ) USAGE;;
  * )
    echo "$0: Unknown instruction: $1"
    USAGE
    ;;
esac

if [[ -f "out/.config" ]]; then
  printf "Do you want to update \'.config\' (y/n)? "
  if check FORCE_RECONFIG; then
    case "$FORCE_RECONFIG" in
      y) echo y; CONFIG;;
      n) echo n;;
    esac
  else
    read RECONFIG
    if [[ "$RECONFIG" == y ]]; then
      CONFIG
    else
      echo "Reconfig is aborted."
    fi
  fi
else
  CONFIG
fi

if [[ -f out/arch/arm/boot/zImage ]]; then
  printf "Do you want to rebuild kernel (y/n)? "
  if check FORCE_REBUILD; then
      case "$FORCE_REBUILD" in
        y) echo y; BUILD;;
        n) echo n;;
      esac
  else
    read REBUILD
    if [[ "$REBUILD" == y ]]; then
      BUILD
    else
      echo "Rebuild is aborted."
    fi
  fi
else
  BUILD
fi

if [[ "$USEZIP" == 1 ]]; then
  printf "Do you want to make installer (y/n)? "
  if check FORCE_INSTALLER; then
    case "$FORCE_INSTALLER" in
      y) echo y; ZIP;;
      n) echo n;;
    esac
  else
    read MAKEINS
    if [[ "$MAKEINS" == y ]]; then
      ZIP
    else
      echo "Zipping is aborted."
    fi
  fi
fi