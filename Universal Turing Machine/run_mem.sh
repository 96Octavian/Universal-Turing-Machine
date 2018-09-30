valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --profile-heap=yes --verbose --log-file=stats/valgrind-out.txt ./build/UTM < $1
