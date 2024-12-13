#
# Sample script for building the Javascript package for MateriaX
# as well as the MaterialX Interactive Viewer and execution of unit tests.
# It is assumed that emscripten and Node have been installed on the system.
#
# The default execution location is assumed to be the root of the MaterialX repository.
# with the emscripten SDK located in an "emsdk" folder under the parent folder
# of the MaterialX repository.

# Default locations
DEFAULT_EMSDK_LOCATION="./emsdk/"
DEFAULT_MATERIALX_LOCATION="."

# Command-line arguments
EMSDK_LOCATION=${1:-$DEFAULT_EMSDK_LOCATION}
MATERIALX_LOCATION=${2:-$DEFAULT_MATERIALX_LOCATION}

# Check if EMSDK_LOCATION with emsdk file exists in the folder
if [ ! -f "$EMSDK_LOCATION/emsdk" ]; then
    echo "Error: EMSDK_LOCATION is not a valid emsdk directory. Exiting script."
    return 1
fi

echo "--------------------- Setup Emscripten ---------------------"
echo "Using EMSDK_LOCATION: $EMSDK_LOCATION"
echo "Using MATERIALX_LOCATION: $MATERIALX_LOCATION"

$EMSDK_LOCATION/emsdk install 2.0.21
$EMSDK_LOCATION/emsdk activate 2.0.21
source $EMSDK_LOCATION/emsdk_env.sh

echo "--------------------- Build MaterialX ---------------------"
cd $MATERIALX_LOCATION

if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]] || [[ "$(uname -s)" == MINGW* || "$(uname -s)" == CYGWIN* ]]; then
    cmake -S . -B javascript/build -DMATERIALX_BUILD_JS=ON -DMATERIALX_EMSDK_PATH=$EMSDK_LOCATION -G Ninja
else
    cmake -S . -B javascript/build -DMATERIALX_BUILD_JS=ON -DMATERIALX_EMSDK_PATH=$EMSDK_LOCATION -G Ninja
fi
if [ $? -ne 0 ]; then
    read -p "Error: cmake configuration failed. Exit or press any key to exit"
    return 1
fi

cmake --build javascript/build --target install --config RelWithDebInfo --parallel 2
if [ $? -ne 0 ]; then
    echo "Error: build failed. Press any key to exit"
    return 1
fi

echo "--------------------- Run JavaScript Tests ---------------------"
pushd .
cd javascript/MaterialXTest
npm install
npm audit fix
npm run test
if [ $? -ne 0 ]; then
    echo "Javascript core tests failed. Exit or press any key to continue"
    return 1
fi
npm run test:browser
if [ $? -ne 0 ]; then
    echo "Javascript browser tests failed. Exit or press any key to continue"
    return 1
fi
popd

echo "--------------------- Run Interactive Viewer ---------------------"
cd javascript/MaterialXView
npm install
npm audit fix
npm run build
if [ $? -ne 0 ]; then
    echo "Viewer build failed. Exit or press any key to continue"
    return 1
fi
npm run start


