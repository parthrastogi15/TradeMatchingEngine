: ${BOOST_ROOT:?"Please define BOOST_ROOT as the path to your boost directory"}

echo "Generating orders. Please wait..."
perl sample_data.pl > ../resources/orders.txt

echo "Building Binary."
make -C ../build -f ../build/Makefile

echo "Performing Test.  Please wait..."
../build/TradingEngine ../resources/orders.txt > ../resources/test.out 2>&1

echo "Test Done! Check the ouptut in /resources/test.out"