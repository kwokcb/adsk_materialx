import argparse
import sys, os

import MaterialX as mx

def main():
    parser = argparse.ArgumentParser(description="Test if two documents are functionally equivalent.")
    parser.add_argument(dest="inputFilename", help="Filename of the input document.")
    parser.add_argument(dest="inputFilename2", help="Filename of the input document to compare against.")
    parser.add_argument('-sa', '--skipAttributes', nargs='+', help="Skip comparisons for attribute. More than one attribute can be specified")
    parser.add_argument('-sv', '--skipValueComparisons', action='store_true', help="Skip value comparisons. Default is to use value comparisons")    
    parser.add_argument('-p', '--precision', type=int, default=None, help="Set the floating precision for comparisons.", )

    opts = parser.parse_args()

    # Check if both files exist
    if not opts.inputFilename or not opts.inputFilename2:
        print("Please provide two filenames to compare.")
        sys.exit(0)
    if not os.path.isfile(opts.inputFilename):
        print(f"File {(opts.inputFilename)} does not exist.")
        sys.exit(0)
    if not os.path.isfile(opts.inputFilename2):
        print(f"File {(opts.inputFilename2)} does not exist.")
        sys.exit(0)

    doc = mx.createDocument()
    try:
        mx.readFromXmlFile(doc, opts.inputFilename)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)

    doc2 = mx.createDocument()
    try:
        mx.readFromXmlFile(doc2, opts.inputFilename2)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)

    print('Version: {}.{}'.format(*doc.getVersionIntegers()))    

    eopts = mx.ElementEquivalenceOptions()
    if opts.skipAttributes:
        for attr in opts.skipAttributes:
            eopts.skipAttributes.add(attr)
    if opts.skipValueComparisons:
        eopts.skipValueComparisons = True
    if opts.precision:
        eopts.precision = opts.precision

    results = mx.ElementEquivalenceResult()
    equivalent = doc.isEquivalent(doc2, eopts)
    equivalent = doc.isEquivalent(doc2, eopts, results)
    if equivalent:
        print(f"Documents are equivalent")
    else:
        print(f"Documents are not equivalent: {results.differenceCount()} differences found")
        for i in range(0, results.differenceCount()):
            difference = results.getDifference(i)
            print(f"- Difference[{i}] : Path: {difference[0]} vs path: {difference[1]}. Difference Type: {difference[2]}"
                  + (f" attribute: {difference[3]}" if difference[3] else "")) 
    
if __name__ == '__main__':
    main()

