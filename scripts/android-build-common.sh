#!/bin/bash -x

SCRIPT_NAME="${BASH_SOURCE[0]}"
SCRIPT_PATH=$(dirname "${BASH_SOURCE[0]}")
SCRIPT_PATH=$(realpath "$SCRIPT_PATH")

FIND_ARGS="-type f -print -quit"
case "$(uname -s)" in

Darwin)
  FIND_ARGS="-perm +111 $FIND_ARGS"
  ;;

*)
  FIND_ARGS="-executable $FIND_ARGS"
  ;;
esac

if [ -z $BUILD_ARCH ]; then
  BUILD_ARCH="armeabi-v7a x86 x86_64 arm64-v8a"
fi

if [ -z $NDK_TARGET ]; then
  NDK_TARGET=21
fi

if [ -z $CMAKE_PROGRAM ]; then
  CMAKE_PROGRAM="cmake-missing"
fi

if [ -z $CCACHE ]; then
  CCACHE=$(which ccache)
fi

if [ -z $ANDROID_NDK ]; then
  ANDROID_NDK="ndk-missing"
fi

if [ -z $ANDROID_SDK ]; then
  ANDROID_SDK="sdk-missing"
fi

if [ -z $BUILD_DST ]; then
  BUILD_DST=$(pwd)/libs
fi

if [ -z $BUILD_SRC ]; then
  BUILD_SRC=$(pwd)/src
fi

if [ -z $SCM_URL ]; then
  SCM_URL="missing"
fi

if [ -z $SCM_TAG ]; then
  SCM_TAG=master
fi

if [ -z $SCM_HASH ]; then
  SCM_HASH="missing"
fi

CLEAN_BUILD_DIR=0

function common_help {
  echo "$SCRIPT_NAME supports the following arguments:"
  echo "	--ndk	The base directory of your android NDK defa"
  echo "			ANDROID_NDK=$ANDROID_NDK"
  echo "	--sdk	The base directory of your android SDK defa"
  echo "			ANDROID_SDK=$ANDROID_SDK"
  echo "	--arch	A list of architectures to build"
  echo "			BUILD_ARCH=$BUILD_ARCH"
  echo "	--dst	The destination directory for include and library files"
  echo "			BUILD_DST=$BUILD_DST"
  echo "	--src	The source directory for SCM checkout"
  echo "			BUILD_SRC=$BUILD_SRC"
  echo "	--url	The SCM source url"
  echo "			SCM_URL=$SCM_URL"
  echo "	--tag	The SCM branch or tag to check out"
  echo "			SCM_TAG=$SCM_TAG"
  echo "	--hash	The SCM commit or hash to check out"
  echo "			SCM_HASH=$SCM_HASH"
  echo "	--clean	Clean the destination before build"
  echo "	--help	Display this help"
  exit 0
}

function common_run {
  echo "[RUN] $@"
  "$@"
  RES=$?
  if [[ $RES -ne 0 ]]; then
    echo "[ERROR] $@ returned $RES"
    exit 1
  fi
}

function common_check_requirements {
  if [[ ! -d $ANDROID_NDK ]]; then
    echo "export ANDROID_NDK to point to your NDK location."
    exit 1
  fi

  if [[ ! -d $ANDROID_SDK ]]; then
    echo "export ANDROID_SDK to point to your SDK location."
    exit 1
  fi
  if [[ -z $BUILD_DST ]]; then
    echo "Destination directory not valid"
    exit 1
  fi

  if [[ -z $BUILD_SRC ]]; then
    echo "Source directory not valid"
    exit 1
  fi

  if [[ -z $SCM_URL ]]; then
    echo "Source URL not defined! Define SCM_URL"
    exit 1
  fi

  if [[ -z $SCM_TAG ]]; then
    echo "SCM_TAG / BRANCH not defined! Define SCM_TAG"
    exit 1
  fi

  if [[ -z $SCM_HASH ]]; then
    echo "SCM_HASH not defined! Define SCM_HASH"
    exit 1
  fi

  if [[ -z $NDK_TARGET ]]; then
    echo "Android platform NDK_TARGET not defined"
    exit 1
  fi

  if [ -z $CMAKE_PROGRAM ] || [ "$CMAKE_PROGRAM" == "cmake-missing" ]; then
    CMAKE_PROGRAM=$(find $ANDROID_SDK/cmake -name cmake $FIND_ARGS)
    if [ -z $CMAKE_PROGRAM ]; then
      echo "CMake not found in $ANDROID_SDK, install CMake from the android SDK!"
      exit 1
    fi
  fi

  for CMD in make git $CMAKE_PROGRAM; do
    if ! type $CMD >/dev/null; then
      echo "Command $CMD not found. Install and add it to the PATH."
      exit 1
    fi
  done

  if [ "${BUILD_SRC:0:1}" != "/" ]; then
    BUILD_SRC=$(pwd)/$BUILD_SRC
  fi
  if [ "${BUILD_DST:0:1}" != "/" ]; then
    BUILD_DST=$(pwd)/$BUILD_DST
  fi
}

function common_parse_arguments {
  while [[ $# > 0 ]]; do
    key="$1"
    case $key in
    --conf)
      source "$2" || exit 1
      shift
      ;;

    --target)
      NDK_TARGET="$2"
      shift
      ;;

    --ndk)
      ANDROID_NDK="$2"
      shift
      ;;

    --sdk)
      ANDROID_SDK="$2"
      shift
      ;;

    --arch)
      BUILD_ARCH="$2"
      shift
      ;;

    --dst)
      BUILD_DST="$2"
      shift
      ;;

    --src)
      BUILD_SRC="$2"
      shift
      ;;

    --url)
      SCM_URL="$2"
      shift
      ;;

    --tag)
      SCM_TAG="$2"
      shift
      ;;

    --hash)
      SCM_HASH="$2"
      shift
      ;;

    --clean)
      CLEAN_BUILD_DIR=1
      shift
      ;;

    --help)
      common_help
      shift
      ;;

    *) # Unknown
      ;;
    esac
    shift
  done

  common_check_requirements
}

function common_update {
  if [ $# -ne 4 ]; then
    echo "Invalid arguments to update function $@"
    exit 1
  fi
  SCM_URL=$1
  SCM_TAG=$2
  BUILD_SRC=$3
  SCM_HASH=$4

  echo "Preparing checkout..."
  BASE=$(pwd)
  CACHE=$(realpath $SCRIPT_PATH/../cache)
  common_run mkdir -p $CACHE
  TARFILE="$CACHE/$SCM_TAG.tar.gz"

  if [[ ! -f "$TARFILE" ]]; then
    WGET=$(which wget)
    CURL=$(which curl)
    if [ -x "$CURL" ]; then
      common_run $CURL -L -o "$TARFILE" "$SCM_URL/$SCM_TAG.tar.gz"
    elif [ -x "$WGET" ]; then
      common_run $WGET -O "$TARFILE" "$SCM_URL/$SCM_TAG.tar.gz"
    else
      echo "Neither wget nor curl installed, aborting"
      exit -1
    fi
  fi

  echo "$SCM_HASH $TARFILE" >$TARFILE.sha256sum
  common_run sha256sum -c $TARFILE.sha256sum

  if [[ -d $BUILD_SRC ]]; then
    common_run rm -rf $BUILD_SRC
  fi
  common_run mkdir -p $BUILD_SRC
  common_run cd $BUILD_SRC
  common_run tar zxf "$TARFILE" --strip 1
  common_run cd $BASE

}

function common_clean {
  if [ $CLEAN_BUILD_DIR -ne 1 ]; then
    return
  fi

  if [ $# -ne 1 ]; then
    echo "Invalid arguments to clean function $@"
    exit 1
  fi

  echo "Cleaning up $1..."
  common_run rm -rf $1
}

function common_copy {
  if [ $# -ne 2 ]; then
    echo "Invalid arguments to copy function $@"
    exit 1
  fi
  if [ ! -d $1 ] || [ ! -d $1/include ] || [ ! -d $1/libs ]; then
    echo "Invalid source $1"
    exit 1
  fi
  if [ -z $2 ]; then
    echo "Invalid destination $2"
    exit 1
  fi

  if [ ! -d $2 ]; then
    common_run mkdir -p $2
  fi
  common_run cp -L -r $1/include $2
  common_run cp -L -r $1/libs/* $2
}
