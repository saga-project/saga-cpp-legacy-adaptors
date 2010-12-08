
# plot directory create test

set terminal postscript
set title 'Directory create test'
set xlabel 'Directory depth'
set ylabel 'Time in seconds'

set output 'directory_create_test.ps'
plot 'directory_create_test.csv' using 1:2

# plot attribute create test
set terminal postscript
set title 'Attribute create test'
set xlabel 'Directory depth'
set ylabel 'Time in seconds'

set output 'attribute_create_test.ps'
plot 'attribute_create_test.csv' using 1:2

# plot directory read test
set terminal postscript
set title 'Directory read test'
set xlabel 'Directory depth'
set ylabel 'Time in seconds'

set output 'directory_read_test.ps'
plot 'directory_read_test.csv' using 1:2

# plot attribute read test
set terminal postscript
set title 'Attribute read test'
set xlabel 'Directory depth'
set ylabel 'Time in seconds'

set output 'attribute_read_test.ps'
plot 'attribute_read_test.csv' using 1:2

