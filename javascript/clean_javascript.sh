#
# Sample script to clean the Javascript package for MaterialX
# It will rebuild the build area for the package as well as remove
# and installed Node modules installed for MaterialXTest and MaterialXView
#
# The default execution location is assumed to be the root of the MaterialX repository.
#
DEFAULT_MATERIALX_LOCATION="."
MATERIALX_LOCATION=${1:-$DEFAULT_MATERIALX_LOCATION}

# Check if MATERIALX_LOCATION with javascript folder exists
if [ ! -d "$MATERIALX_LOCATION/javascript" ]; then
    echo "Error: MATERIALX_LOCATION is not a valid MaterialX directory"
    exit 1
fi
pushd .
cd $MATERIALX_LOCATION/javascript
echo "Remove build folder" 
rm -rf build 
echo "Remove MaterialXTest build folder"
rm -rf MaterialXTest/_build 
echo "Remove MaterialXTest  node modules folder"
rm -rf MaterialXTest/node_modules 
echo "Remove MaterialXView dist folder"
rm -rf MaterialXView/dist 
echo "Remove MaterialXView node modules folder"
rm -rf MaterialXView/node_modules 
popd