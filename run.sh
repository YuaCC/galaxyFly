#python galaxy.py 11 29 1 10 > anynet_file
#./booksim2-master/src/booksim log/anynet_config_11_29_1_10_load6 > log/anynet_config_11_29_1_10_load6.txt

#python galaxy.py 15 29 1 7 > anynet_file
#./booksim2-master/src/booksim log/anynet_config_15_29_1_7_load8 > log/anynet_config_15_29_1_7_load8.txt
#
#python galaxy.py 81 1 10 4 > anynet_file
#./booksim2-master/src/booksim log/anynet_config_81_1_10_4_load8 > log/anynet_config_81_1_10_4_load8.txt
#
#python galaxy.py 15 29 4 2 > anynet_file
#./booksim2-master/src/booksim log/anynet_config_15_29_4_2_load5 > log/anynet_config_15_29_4_2_load5.txt

python galaxy.py 11 29 1 10 > galaxyfly_topology
./booksim2-master/src/booksim log/galaxyfly_config_11_29_1_10_load6 > log/galaxyfly_config_11_29_1_10_load6.txt

#python galaxy.py 15 29 1 7 > galaxyfly_topology
#./booksim2-master/src/booksim log/galaxyfly_config_15_29_1_7_load8 > log/galaxyfly_config_15_29_1_7_load8.txt
#
#python galaxy.py 15 29 4 2 > galaxyfly_topology
#./booksim2-master/src/booksim log/galaxyfly_config_15_29_4_2_load5 > log/galaxyfly_config_15_29_4_2_load5.txt
#
#python galaxy.py 81 1 10 4 > galaxyfly_topology
#./booksim2-master/src/booksim log/galaxyfly_config_81_1_10_4_load8 > log/galaxyfly_config_81_1_10_4_load8.txt
