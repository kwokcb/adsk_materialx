source /Users/bernardkwok/work/emsdk/emsdk install latest
source /Users/bernardkwok/work/emsdk/emsdk activate latest
source /Users/bernardkwok/work/emsdk/emsdk_env.sh

mkdir javascript/build
cd javascript/build
cmake -DMATERIALX_BUILD_JS=ON -DMATERIALX_EMSDK_PATH="/Users/bernardkwok/work/emsdk" -G Ninja -DMATERIALX_BUILD_RENDER=OFF -DMATERIALX_BUILD_TESTS=OFF -DMATERIALX_BUILD_GEN_OSL=OFF -DMATERIALX_BUILD_GEN_MDL=OFF ../..
cmake --build . --target install --config  RelWithDebInfo --parallel 2

cd ../MaterialXTest
npm install
npm run test
npm run test:browser

cd ../MaterialXView
npm install
npm run build
npm install http-server -g
http-server . -p 8000
