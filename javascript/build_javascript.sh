
# Default locations
DEFAULT_EMSDK_LOCATION="./emsdk/"
DEFAULT_MATERIALX_LOCATION="."

# Command-line arguments
EMSDK_LOCATION=${1:-$DEFAULT_EMSDK_LOCATION}
MATERIALX_LOCATION=${2:-$DEFAULT_MATERIALX_LOCATION}

# Check if EMSDK_LOCATION with emsdk file exists in the folder
if [ ! -f "$EMSDK_LOCATION/emsdk" ]; then
    read -p "Error: EMSDK_LOCATION is not a valid emsdk directory. Press any key to exit"
    exit 1
fi

echo "--------------------- Setup Emscripten ---------------------"
echo "Using EMSDK_LOCATION: $EMSDK_LOCATION"
echo "Using MATERIALX_LOCATION: $MATERIALX_LOCATION"

$EMSDK_LOCATION/emsdk install 2.0.20
$EMSDK_LOCATION/emsdk activate 2.0.20

echo "--------------------- Build MaterialX ---------------------"
cd $MATERIALX_LOCATION
cmake -S . -B javascript/build -DMATERIALX_BUILD_JS=ON -DMATERIALX_EMSDK_PATH=$EMSDK_LOCATION -G Ninja
if [ $? -ne 0 ]; then
    read -p "Error: cmake configuration failed. Exit or press any key to exit"
    exit 1
fi

cmake --build javascript/build --target install --config RelWithDebInfo --parallel 2
if [ $? -ne 0 ]; then
    read -p "Error: build failed. Press any key to exit"
    exit 1
fi

echo "--------------------- Run JavaScript Tests ---------------------"
cd javascript/MaterialXTest
npm install
npm audit fix
npm run test
if [ $? -ne 0 ]; then
    read -p "Javascript core tests failed. Exit or press any key to continue"
fi
npm run test:browser
if [ $? -ne 0 ]; then
    read -p "Javascript browser tests failed. Exit or press any key to continue"
fi
cd ..

echo "--------------------- Run Interactive Viewer ---------------------"
cd javascript/MaterialXView
npm install
npm audit fix
npm run build
if [ $? -ne 0 ]; then
    read -p "Viewer build failed. Exit or press any key to continue"
fi
npm run start

